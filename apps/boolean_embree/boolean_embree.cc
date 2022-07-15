#include <embree3/rtcore.h>
#include <vector>
#include <iostream>
#include <limits>

#include "stl_io.hh"
#include "boolean_embree.hh"
#include "../timers.hh"

using namespace mp::io;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: boolean4 a.stl b.stl" << std::endl;
        return 1;
    }

    const char *filepath_1 = argv[1];
    const char *filepath_2 = argv[2];

    RTCDevice device = rtcNewDevice(NULL);
    RTCScene scene = rtcNewScene(device);

    // Data MUST be allocated on heap to be shared between threads
    IntersectionData *data = new IntersectionData{};

    stl::read_stl(filepath_1, data->tri_soup);
    stl::read_stl(filepath_2, data->tri_soup);

    // TODO:
    // 1) deduplicate vertices and build topology from triangles
    // 2) build two bvhs, one for edges, and one for triangles and overlap them (maybe not good idea, twice build time?)

    data->intersection_points.reserve(data->tri_soup.size());

    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_USER);
    unsigned int geomID = rtcAttachGeometry(scene, geom);
    rtcSetGeometryUserPrimitiveCount(geom, data->tri_soup.size());
    rtcSetGeometryUserData(geom, data);
    rtcSetGeometryBoundsFunction(geom, triangle_bounds_func, nullptr);
    rtcSetGeometryIntersectFunction(geom, triangle_intersect_func);
    rtcCommitGeometry(geom);
    rtcReleaseGeometry(geom);

    /*
     * Like geometry objects, scenes must be committed. This lets
     * Embree know that it may start building an acceleration structure.
     */
    rtcCommitScene(scene);

    // Perform self intersection
    Timer timer;
    rtcCollide(scene, scene, collide_func, data);
    timer.tock("rtcCollide");

    // You can try to get rid of the map of vectors, and sort the intersection points instead,
    // and triangulate in a linear scan, but the logic is more complicated
    // timer.tick();
    // std::sort(data->intersection_points.begin(), data->intersection_points.end(), [](const IntersectionPoint &a, const IntersectionPoint &b)
    //           { return a.primID < b.primID; });
    // timer.tock("Sorting intersection points by primtivie id");

    // for (int i = 0; (i < 100) && (i < data->intersection_points.size()); i++)
    // {
    //     auto ip = data->intersection_points[i];
    //     std::cout << ip.p << " " << ip.primID << std::endl;
    // }

    std::vector<stl::Triangle> out;
    stl::Triangle tri_buf;
    timer.tick();
    for (const auto &it : data->intersection_points_map)
    {
        std::vector<std::pair<Triangulation::Point, TriangulationPointInfo>> triangulation_input;
        const auto triangle = to_cgal_triangle(data->tri_soup[it.first]);
        for (int i = 0; i < 3; i++)
        {
            const auto &p3d = triangle.vertex(i);
            auto p2d = project_point(p3d, triangle);
            TriangulationPointInfo info{p3d};
            triangulation_input.push_back({p2d, info});
        }
        for (const auto &ip : it.second)
        {
            const auto &p3d = ip.p;
            auto p2d = project_point(p3d, triangle);
            TriangulationPointInfo info{p3d};
            triangulation_input.push_back({p2d, info});
        }

        Triangulation triangulation(triangulation_input.begin(), triangulation_input.end());
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

    std::cout << out.size() << std::endl;
    stl::write_stl(out, "foo.stl");

    rtcReleaseScene(scene);
    rtcReleaseDevice(device);
    delete data;

    return 0;
}
