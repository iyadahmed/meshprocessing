#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdio>
#include <embree3/rtcore.h>

#include "stl_io.hh"
#include "vec3.hh"
#include "../timers.hh"

#include "../monte_carlo_full_sphere_samples.hh"
#include "embree_do_intersect.hh"
#include "embree_num_intersections.hh"
#include "embree_device.hh"
#include "embree_scene.hh"

using namespace mp::io::stl;

static bool is_inside(const RTCScene &scene, const float &x, const float &y, const float &z)
{
    int odd_intersections_num = 0;
    for (int i = 0; i < NUM_SPHERE_SAMPLES; i++)
    {
        int n = num_intersections(scene, x, y, z, SPHERE_SAMPLES[i][0], SPHERE_SAMPLES[i][1], SPHERE_SAMPLES[i][2]);
        odd_intersections_num += (n & 1); // if odd add 1, if even add 0
    }
    return (odd_intersections_num) >= (.5 * NUM_SPHERE_SAMPLES);
}

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
                if (is_inside(scene, x, y, z))
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
