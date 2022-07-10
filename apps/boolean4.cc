#include <embree3/rtcore.h>
#include <vector>
#include <iostream>
#include <limits>

#include "stl_io.hh"
#include "boolean4.hh"
#include "timers.hh"

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
    Data *data = new Data{};

    stl::read_stl(filepath_1, data->tri_soup);
    stl::read_stl(filepath_2, data->tri_soup);

    data->intersections.reserve(data->tri_soup.size());

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

    std::cout << data->intersections.size() << std::endl;

    rtcReleaseScene(scene);
    rtcReleaseDevice(device);
    delete data;

    return 0;
}
