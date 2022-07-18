#include <embree3/rtcore.h>
#include <vector>
#include <execution>
#include <iostream>
#include <limits>
#include <fstream>

#include "stl_io.hh"
#include "boolean_embree.hh"
#include "indexed_mesh.hh"
#include "timers.hh"

using namespace mp::io;

static inline void write_point(std::ofstream &file, const Point &p)
{
    double x = CGAL::to_double(p.x());
    double y = CGAL::to_double(p.y());
    double z = CGAL::to_double(p.z());
    file.write((char *)(&x), sizeof(double));
    file.write((char *)(&y), sizeof(double));
    file.write((char *)(&z), sizeof(double));
}

static inline stl::Triangle to_stl_triangle(Triangulation::Finite_faces_iterator &it)
{
    stl::Triangle out;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            out.verts[i][j] = CGAL::to_double(it->vertex(i)->info().point_3d[j]);
        }
    }
    return out;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: boolean_embree a.stl b.stl" << std::endl;
        return 1;
    }

    const char *filepath_1 = argv[1];
    const char *filepath_2 = argv[2];

    RTCDevice device = rtcNewDevice(NULL);
    RTCScene scene = rtcNewScene(device);

    // Data MUST be allocated on heap to be shared between threads
    IntersectionData *data = new IntersectionData;

    stl::read_stl(filepath_1, data->tri_soup);
    stl::read_stl(filepath_2, data->tri_soup);
    std::cout << "Number of Input Triangles = " << data->tri_soup.size() << std::endl;

    data->cgal_tris.reserve(data->tri_soup.size());
    for (const auto &t : data->tri_soup)
    {
        auto t_cgal = to_cgal_triangle(t);
        if (!t_cgal.is_degenerate()) // Only keep non-degenerate triangles
        {
            Segment s1 = {t_cgal.vertex(0), t_cgal.vertex(1)};
            Segment s2 = {t_cgal.vertex(1), t_cgal.vertex(2)};
            Segment s3 = {t_cgal.vertex(2), t_cgal.vertex(0)};
            data->cgal_tris.push_back({t_cgal, s1, s2, s3});
        }
    }

    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_USER);
    unsigned int geomID = rtcAttachGeometry(scene, geom);
    rtcSetGeometryUserPrimitiveCount(geom, data->tri_soup.size());
    rtcSetGeometryUserData(geom, data);
    rtcSetGeometryBoundsFunction(geom, triangle_bounds_func_cgal_tris, nullptr);
    // TODO: set ray triangle intersection function
    // rtcSetGeometryIntersectFunction(geom, triangle_intersect_func);
    rtcCommitGeometry(geom);
    rtcReleaseGeometry(geom);

    /*
     * Like geometry objects, scenes must be committed. This lets
     * Embree know that it may start building an acceleration structure.
     */
    rtcCommitScene(scene);

    // Self intersection
    Timer timer;
    rtcCollide(scene, scene, collide_func_cgal_tris, data);
    timer.tock("Calculating self-intersection points");

    // Triangulation
    timer.tick();
    std::vector<stl::Triangle> out;
    stl::Triangle tri_buf;
    for (auto &prim_id_intersection_points_pair : data->intersection_points_map)
    {
        const auto &t = data->cgal_tris[prim_id_intersection_points_pair.first].t;
        // Include original triangle points in the triangulation
        // not just intersecion points
        for (int i = 0; i < 3; i++)
        {
            auto p = t.vertex(i);
            TriangulationPointInfo info{p};
            auto p2d = project_point(p, t);
            prim_id_intersection_points_pair.second.push_back({p2d, info});
        }
        Triangulation triangulation(prim_id_intersection_points_pair.second.begin(), prim_id_intersection_points_pair.second.end());
        for (auto it = triangulation.finite_faces_begin(); it != triangulation.finite_faces_end(); it++)
        {
            out.push_back(to_stl_triangle(it));
        }
    }
    timer.tock("Triangulation");

    std::cout << "Number of output triangles = " << out.size() << std::endl;
    stl::write_stl(out, "boolean_embree_output.stl");

    rtcReleaseScene(scene);
    rtcReleaseDevice(device);
    delete data;

    return 0;
}
