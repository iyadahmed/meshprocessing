#include <embree3/rtcore.h>
#include <vector>
#include <execution>
#include <iostream>
#include <limits>

#include "stl_io.hh"
#include "boolean_embree.hh"
#include "indexed_mesh.hh"
#include "timers.hh"

using namespace mp::io;

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
        if (!t_cgal.is_degenerate())
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
    // rtcSetGeometryIntersectFunction(geom, triangle_intersect_func);
    rtcCommitGeometry(geom);
    rtcReleaseGeometry(geom);

    /*
     * Like geometry objects, scenes must be committed. This lets
     * Embree know that it may start building an acceleration structure.
     */
    rtcCommitScene(scene);

    // Perform self intersection
    Timer timer;
    rtcCollide(scene, scene, collide_func_cgal_tris, data);
    timer.tock("Calculating intersection points");

    // std::ofstream file("boolean_embree_intersection_points.pts", std::ios::binary);
    // for (const auto &ip : data->intersection_points)
    // {
    //     double x = CGAL::to_double(ip.p.x());
    //     double y = CGAL::to_double(ip.p.y());
    //     double z = CGAL::to_double(ip.p.z());
    //     file.write((char *)(&x), sizeof(double));
    //     file.write((char *)(&y), sizeof(double));
    //     file.write((char *)(&z), sizeof(double));
    // }

    timer.tick();
    std::vector<stl::Triangle> out;
    stl::Triangle tri_buf;
    for (auto &prim_id_intersection_points_pair : data->intersection_points_map)
    {
        const auto &t = data->cgal_tris[prim_id_intersection_points_pair.first].t;
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
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    tri_buf.verts[i][j] = CGAL::to_double(it->vertex(i)->info().point_3d[j]);
                }
            }
            out.push_back(tri_buf);
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
