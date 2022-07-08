#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdio>

#include "stl_io.hh"
#include "vec3.hh"
#include "timers.hh"

#include <embree3/rtcore.h>

#include "mcpip_embree.hh"

/*
 * This is only required to make the tutorial compile even when
 * a custom namespace is set.
 */
#if defined(RTC_NAMESPACE_USE)
RTC_NAMESPACE_USE
#endif

/*
 * We will register this error handler with the device in initializeDevice(),
 * so that we are automatically informed on errors.
 * This is extremely helpful for finding bugs in your code, prevents you
 * from having to add explicit error checking to each Embree API call.
 */
void errorFunction(void *userPtr, enum RTCError error, const char *str)
{
    printf("error %d: %s\n", error, str);
}

/*
 * Embree has a notion of devices, which are entities that can run
 * raytracing kernels.
 * We initialize our device here, and then register the error handler so that
 * we don't miss any errors.
 *
 * rtcNewDevice() takes a configuration string as an argument. See the API docs
 * for more information.
 *
 * Note that RTCDevice is reference-counted.
 */
RTCDevice initializeDevice()
{
    RTCDevice device = rtcNewDevice(NULL);

    if (!device)
        printf("error %d: cannot create device\n", rtcGetDeviceError(NULL));

    rtcSetDeviceErrorFunction(device, errorFunction, NULL);
    return device;
}

using namespace mp::io::stl;

/*
 * Create a scene, which is a collection of geometry objects. Scenes are
 * what the intersect / occluded functions work on. You can think of a
 * scene as an acceleration structure, e.g. a bounding-volume hierarchy.
 *
 * Scenes, like devices, are reference-counted.
 */
RTCScene initializeScene(RTCDevice device, const std::vector<Triangle> &tris)
{
    RTCScene scene = rtcNewScene(device);

    /*
     * Create a triangle mesh geometry, and initialize a single triangle.
     * You can look up geometry types in the API documentation to
     * find out which type expects which buffers.
     *
     * We create buffers directly on the device, but you can also use
     * shared buffers. For shared buffers, special care must be taken
     * to ensure proper alignment and padding. This is described in
     * more detail in the API documentation.
     */

    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
    float *vertices = (float *)rtcSetNewGeometryBuffer(geom,
                                                       RTC_BUFFER_TYPE_VERTEX,
                                                       0,
                                                       RTC_FORMAT_FLOAT3,
                                                       3 * sizeof(float),
                                                       tris.size() * 3);

    unsigned *indices = (unsigned *)rtcSetNewGeometryBuffer(geom,
                                                            RTC_BUFFER_TYPE_INDEX,
                                                            0,
                                                            RTC_FORMAT_UINT3,
                                                            3 * sizeof(unsigned),
                                                            tris.size());

    if (vertices && indices)
    {
        int vi = 0;
        int ii = 0;
        for (const auto &t : tris)
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

    return scene;
}

/*
 * Cast a single ray with origin (ox, oy, oz) and direction
 * (dx, dy, dz).
 */
void castRay(RTCScene scene,
             float ox, float oy, float oz,
             float dx, float dy, float dz)
{
    /*
     * The intersect context can be used to set intersection
     * filters or flags, and it also contains the instance ID stack
     * used in multi-level instancing.
     */
    struct RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    /*
     * The ray hit structure holds both the ray and the hit.
     * The user must initialize it properly -- see API documentation
     * for rtcIntersect1() for details.
     */
    struct RTCRayHit rayhit;
    rayhit.ray.org_x = ox;
    rayhit.ray.org_y = oy;
    rayhit.ray.org_z = oz;
    rayhit.ray.dir_x = dx;
    rayhit.ray.dir_y = dy;
    rayhit.ray.dir_z = dz;
    rayhit.ray.tnear = 0;
    rayhit.ray.tfar = std::numeric_limits<float>::infinity();
    rayhit.ray.mask = -1;
    rayhit.ray.flags = 0;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

    /*
     * There are multiple variants of rtcIntersect. This one
     * intersects a single ray with the scene.
     */
    rtcIntersect1(scene, &context, &rayhit);

    printf("%f, %f, %f: ", ox, oy, oz);
    if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
    {
        /* Note how geomID and primID identify the geometry we just hit.
         * We could use them here to interpolate geometry information,
         * compute shading, etc.
         * Since there is only a single triangle in this scene, we will
         * get geomID=0 / primID=0 for all hits.
         * There is also instID, used for instancing. See
         * the instancing tutorials for more information */
        printf("Found intersection on geometry %d, primitive %d at tfar=%f\n",
               rayhit.hit.geomID,
               rayhit.hit.primID,
               rayhit.ray.tfar);
    }
    else
        printf("Did not find any intersection.\n");
}

bool do_intersect(const RTCScene &scene, float ox, float oy, float oz,
                  float dx, float dy, float dz)
{
    /*
     * The intersect context can be used to set intersection
     * filters or flags, and it also contains the instance ID stack
     * used in multi-level instancing.
     */
    struct RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    struct RTCRay ray;
    ray.org_x = ox;
    ray.org_y = oy;
    ray.org_z = oz;
    ray.dir_x = dx;
    ray.dir_y = dy;
    ray.dir_z = dz;
    ray.tnear = 0;
    ray.tfar = std::numeric_limits<float>::infinity();
    ray.mask = -1;
    ray.flags = 0;

    /*
     * When no intersection is found,
     * the ray data is not updated.
     * In case a hit was found,
     * the tfar component of the ray is set to -inf.
     */
    rtcOccluded1(scene, &context, &ray);

    return ray.tfar == -std::numeric_limits<float>::infinity();
}

// static bool is_inside(const RTCScene &scene, const float &x, const float &y, const float &z)
// {
//     int odd_intersections_num = 0;
//     int non_zero_intersections_num = 0;
//     for (int i = 0; i < NUM_SPHERE_SAMPLES; i++)
//     {
//         size_t n = tree.number_of_intersected_primitives<CGALRay3>({query_point, SPHERE_SAMPLES[i]});
//         non_zero_intersections_num += (bool)n;
//         odd_intersections_num += (n & 1);
//     }

//     return (odd_intersections_num >= (.5 * float(non_zero_intersections_num)))

//            && (non_zero_intersections_num >= (.4 * float(sizeof(SPHERE_SAMPLES) / sizeof(SPHERE_SAMPLES[0]))));
// }

static bool is_inside_no_holes(const RTCScene &scene, const float &x, const float &y, const float &z)
{
    for (int i = 0; i < NUM_SPHERE_SAMPLES; i++)
    {

        if (!do_intersect(scene, x, y, z, SPHERE_SAMPLES[i][0], SPHERE_SAMPLES[i][1], SPHERE_SAMPLES[i][2]))
        {
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        puts("Monte Carlo Point in Polygon 3D\n"
             "Usage: mcpip grid_step input_filepath.stl output_filepath.pts\n"
             "Generates points inside the volume of an oriented triangle soup by filtering bounding box grid points.\n"
             "Outputs a binary file containing N * 3 doubles.");
        return 1;
    }

    float grid_step = atof(argv[1]);
    if (grid_step <= 0.0f)
    {
        puts("ERROR: Grid step must be a positive number.");
        return 1;
    }
    char *input_filepath = argv[2];
    char *output_filepath = argv[3];

    std::vector<Triangle> tris;
    read_stl(input_filepath, tris);

    /* Initialization. All of this may fail, but we will be notified by
     * our errorFunction. */
    RTCDevice device = initializeDevice();
    RTCScene scene = initializeScene(device, tris);

    // Calculate bounding box
    Vec3 bb_min(INFINITY, INFINITY, INFINITY);
    Vec3 bb_max(-INFINITY, -INFINITY, -INFINITY);
    for (auto const &tri : tris)
    {
        for (int i = 0; i < 3; i++)
        {
            auto v = tri.verts[i];
            auto vec = Vec3(v[0], v[1], v[2]);
            bb_min.min(vec);
            bb_max.max(vec);
        }
    }

    // Generate and filter grid points and write them to a file
    std::ofstream file(output_filepath, std::ios::binary);

    Vec3 bb_dims = bb_max - bb_min;
    int num_x = static_cast<int>(bb_dims.x / grid_step);
    int num_y = static_cast<int>(bb_dims.y / grid_step);
    int num_z = static_cast<int>(bb_dims.z / grid_step);

    int num_points = num_x * num_y * num_z;
    printf("Number of grid points before filtering = %d\n", num_points);

    Timer timer;
#pragma omp parallel for collapse(3)
    for (int i = 0; i < num_x; i++)
    {
        for (int j = 0; j < num_y; j++)
        {
            for (int k = 0; k < num_z; k++)
            {
                float x = i * grid_step + bb_min.x;
                float y = j * grid_step + bb_min.y;
                float z = k * grid_step + bb_min.z;
                if (is_inside_no_holes(scene, x, y, z))
                {
#pragma omp critical
                    {
                        file.write(reinterpret_cast<const char *>(&x), sizeof(float));
                        file.write(reinterpret_cast<const char *>(&y), sizeof(float));
                        file.write(reinterpret_cast<const char *>(&z), sizeof(float));
                    }
                }
            }
        }
    }
    timer.tock("Filtering points");

    rtcReleaseScene(scene);
    rtcReleaseDevice(device);

    return 0;
}
