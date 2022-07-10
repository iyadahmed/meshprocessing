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
typedef CGAL::Exact_predicates_exact_constructions_kernel K;

// typedef CGAL::Simple_cartesian<double> K; // Faster but misses intersections
typedef K::Point_3 Point;
typedef K::Triangle_3 Triangle;
typedef K::Segment_3 Segment;
typedef K::Vector_3 Vector;

#include "stl_io.hh"
#include "boolean4.hh"
#include "timers.hh"

using namespace mp::io;

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
