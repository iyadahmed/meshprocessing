#include <embree3/rtcore.h>
#include <vector>
#include <iostream>
#include <limits>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

// Use exact predicates and constructions to avoid precondition exception (degenerate edges being generated while intersecting triangles)
// Also for better precision and handling coplanar cases
// typedef CGAL::Exact_predicates_exact_constructions_kernel K;

typedef CGAL::Simple_cartesian<double> K; // Faster but misses intersections
typedef K::Point_3 Point;
typedef K::Triangle_3 Triangle;
typedef K::Segment_3 Segment;
typedef K::Vector_3 Vector;

#include "stl_io.hh"
#include "boolean4.hh"
#include "timers.hh"

using namespace mp::io;

static Triangle to_cgal_triangle(const stl::Triangle &t)
{
    return {
        {t.v1[0], t.v1[1], t.v1[2]},
        {t.v2[0], t.v2[1], t.v2[2]},
        {t.v3[0], t.v3[1], t.v3[2]},
    };
}

static bool intersect_triangle_triangle(const std::vector<stl::Triangle> &tri_soup, unsigned geomID0, unsigned primID0, unsigned geomID1, unsigned primID1)
{
    if (primID0 == primID1)
    {
        return false;
    }

    const stl::Triangle &t1 = tri_soup[primID0];
    const stl::Triangle &t2 = tri_soup[primID1];
    return CGAL::do_intersect(to_cgal_triangle(t1), to_cgal_triangle(t2));
}

static void collide_func(void *user_data_ptr, RTCCollision *collisions, unsigned int num_collisions)
{
    for (size_t i = 0; i < num_collisions;)
    {
        bool intersect = intersect_triangle_triangle(((Data *)user_data_ptr)->tri_soup,
                                                     collisions[i].geomID0, collisions[i].primID0,
                                                     collisions[i].geomID1, collisions[i].primID1);
        if (intersect)
            i++;
        else
            collisions[i] = collisions[--num_collisions];
    }

    if (num_collisions == 0)
        return;

    // TODO: collect intersections
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: boolean3 a.stl b.stl" << std::endl;
        return 1;
    }

    const char *filepath_1 = argv[1];
    const char *filepath_2 = argv[2];

    RTCDevice device = rtcNewDevice(NULL);
    RTCScene scene = rtcNewScene(device);

    // Data MUST be allocated on heap to be shared between threads
    Data *data_ptr = new Data{};

    stl::read_stl(filepath_1, data_ptr->tri_soup);
    stl::read_stl(filepath_2, data_ptr->tri_soup);

    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_USER);
    unsigned int geomID = rtcAttachGeometry(scene, geom);
    rtcSetGeometryUserPrimitiveCount(geom, data_ptr->tri_soup.size());
    rtcSetGeometryUserData(geom, data_ptr);
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
    rtcCollide(scene, scene, collide_func, data_ptr);
    timer.tock("rtcCollide");

    rtcReleaseScene(scene);
    rtcReleaseDevice(device);

    return 0;
}
