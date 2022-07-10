#include <embree3/rtcore.h>
#include <vector>
#include <iostream>
#include <limits>

#include "stl_io.hh"

using namespace mp::io;

static void collide_func(void *user_data_ptr, RTCCollision *collisions, int num_collisions)
{
    // TODO
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

    std::vector<stl::Triangle> tri_soup;
    stl::read_stl(filepath_1, tri_soup);
    stl::read_stl(filepath_2, tri_soup);

    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
    float *vertices = (float *)rtcSetNewGeometryBuffer(geom,
                                                       RTC_BUFFER_TYPE_VERTEX,
                                                       0,
                                                       RTC_FORMAT_FLOAT3,
                                                       3 * sizeof(float),
                                                       tri_soup.size() * 3);

    unsigned *indices = (unsigned *)rtcSetNewGeometryBuffer(geom,
                                                            RTC_BUFFER_TYPE_INDEX,
                                                            0,
                                                            RTC_FORMAT_UINT3,
                                                            3 * sizeof(unsigned),
                                                            tri_soup.size());

    if (!(vertices && indices))
    {
        std::cerr << "Failed to create Embree geometry." << std::endl;
        return 1;
    }

    int vi = 0;
    int ii = 0;
    for (const auto &t : tri_soup)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                vertices[vi] = t.verts[i][j];
                vi += 1;
            }
            indices[ii] = ii;
            ii += 1;
        }
    }

    /*
     * You must commit geometry objects when you are done setting them up,
     * or you will not get any intersections.
     */
    rtcCommitGeometry(geom);

    /*
     * In rtcAttachGeometry(...), the scene takes ownership of the geom
     * by increasing its reference count. This means that we don't have
     * to hold on to the geom handle, and may release it. The geom object
     * will be released automatically when the scene is destroyed.
     *
     * rtcAttachGeometry() returns a geometry ID. We could use this to
     * identify intersected objects later on.
     */
    rtcAttachGeometry(scene, geom);
    rtcReleaseGeometry(geom);

    /*
     * Like geometry objects, scenes must be committed. This lets
     * Embree know that it may start building an acceleration structure.
     */
    rtcCommitScene(scene);

    rtcReleaseScene(scene);
    rtcReleaseDevice(device);

    return 0;
}
