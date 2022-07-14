#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdio>
#include <embree3/rtcore.h>
#include <algorithm>
#include <execution>

#include "stl_io.hh"
#include "vec3.hh"
#include "../timers.hh"

#include "../monte_carlo_full_sphere_samples.hh"
#include "embree_do_intersect.hh"
#include "embree_num_intersections.hh"
#include "embree_device.hh"
#include "embree_scene.hh"

using namespace mp::io::stl;

static bool is_inside(const RTCScene &scene, const float &x, const float &y, const float &z, const float &threshold)
{
    int odd_intersections_num = 0;
    for (int i = 0; i < NUM_SPHERE_SAMPLES; i++)
    {
        int n = num_intersections(scene, x, y, z, SPHERE_SAMPLES[i][0], SPHERE_SAMPLES[i][1], SPHERE_SAMPLES[i][2]);
        odd_intersections_num += (n & 1); // if odd add 1, if even add 0
    }
    return (odd_intersections_num) >= (threshold * NUM_SPHERE_SAMPLES);
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        puts("Monte Carlo Point in Polygon 3D\n"
             "Usage: mcpip_embree input_filepath.stl output_filepath.pts grid_step threshold\n"
             "Generates points inside the volume of an oriented triangle soup by filtering bounding box grid points.\n"
             "Outputs a binary file containing N * 3 floats.");
        return 1;
    }

    char *input_filepath = argv[1];
    char *output_filepath = argv[2];
    float grid_step = atof(argv[3]);
    if (grid_step <= 0.0f)
    {
        puts("ERROR: Grid step must be a positive number.");
        return 1;
    }
    float threshold = atof(argv[4]);
    if ((threshold > 1.0f) || (threshold < 0.0f))
    {
        puts("ERROR: Threshold must be between 0.0 and 1.0 inclusive.");
        return 1;
    }

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

    // Generate grid points filter them and write inside points to a file
    Vec3 bb_dims = bb_max - bb_min;
    int num_x = std::ceil(bb_dims.x / grid_step);
    int num_y = std::ceil(bb_dims.y / grid_step);
    int num_z = std::ceil(bb_dims.z / grid_step);

    int num_points = num_x * num_y * num_z;
    printf("Number of grid points before filtering = %d\n", num_points);

    std::vector<std::pair<Vec3, bool>> points;
    points.reserve(num_points);
    for (int i = 0; i < num_x; i++)
    {
        for (int j = 0; j < num_y; j++)
        {
            for (int k = 0; k < num_z; k++)
            {
                float x = i * grid_step + bb_min.x;
                float y = j * grid_step + bb_min.y;
                float z = k * grid_step + bb_min.z;
                points.push_back(std::make_pair(Vec3{x, y, z}, false));
            }
        }
    }

    auto func = [&](std::pair<Vec3, bool> &item)
    { item.second = is_inside(scene, item.first.x, item.first.y, item.first.z, threshold); };

    Timer timer;
    std::for_each(std::execution::par_unseq, points.begin(), points.end(), func);
    timer.tock("Filtering points");

    std::ofstream file(output_filepath, std::ios::binary);
    for (const auto &p : points)
    {
        if (p.second)
        {
            file.write((char *)(&p.first.x), sizeof(float));
            file.write((char *)(&p.first.y), sizeof(float));
            file.write((char *)(&p.first.z), sizeof(float));
        }
    }

    rtcReleaseScene(scene);
    rtcReleaseDevice(device);

    return 0;
}
