#include <cstdio>
#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>

#include "stl_io.hh"
#include "vec3.hh"

using namespace mp::io::stl;

static double tet_solid_angle(const Vec3 &origin, Vec3 a, Vec3 b, Vec3 c)
{
    a = a - origin;
    b = b - origin;
    c = c - origin;

    auto al = a.length();
    auto bl = b.length();
    auto cl = c.length();

    auto numerator = a.dot(b.cross(c));
    auto denominator = al * bl * cl + a.dot(b) * cl + a.dot(c) * bl + b.dot(c) * al;

    return atan2(numerator, denominator);
}

static double calc_winding_number(const Vec3 &query_point, const std::vector<Triangle> &tris)
{
    double w = 0.0;
    for (const auto &t : tris)
    {
        w += tet_solid_angle(query_point, t.v1, t.v2, t.v3);
    }
    return w;
}

static bool is_inside(const Vec3 &query_point, const std::vector<Triangle> &tris)
{
    return calc_winding_number(query_point, tris) >= (2.0 * M_PI);
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        puts("Usage: winding_numbers input_filepath.stl grid_step output_filepath.pts\n"
             "Generates points inside the volume of an oriented triangle soup by filtering bounding box grid points.\n"
             "Outputs a binary file containing N * 3 floats.");
        return 1;
    }

    char *input_filepath = argv[1];
    float grid_step = atof(argv[2]);
    if (grid_step <= 0.0f)
    {
        puts("ERROR: Grid step must be a positive number.");
        return 1;
    }
    char *output_filepath = argv[3];

    // Load mesh
    std::vector<Triangle> mesh;
    read_stl(input_filepath, mesh);

    // Calculate bounding box
    Vec3 bb_min(INFINITY, INFINITY, INFINITY);
    Vec3 bb_max(-INFINITY, -INFINITY, -INFINITY);
    for (auto const &tri : mesh)
    {
        for (int i = 0; i < 3; i++)
        {
            auto vec = Vec3(tri.verts[i]);
            bb_min.min(vec);
            bb_max.max(vec);
        }
    }

    std::ofstream file(output_filepath, std::ios::binary);

    Vec3 query_point = bb_min;
    Vec3 bb_dims = bb_max - bb_min;
    int num_x = static_cast<int>(bb_dims.x / grid_step);
    int num_y = static_cast<int>(bb_dims.y / grid_step);
    int num_z = static_cast<int>(bb_dims.z / grid_step);
    for (int i = 0; i < num_x; i++)
    {
        for (int j = 0; j < num_y; j++)
        {
            for (int k = 0; k < num_z; k++)
            {
                if (is_inside(query_point, mesh))
                {
                    file.write(reinterpret_cast<char *>(&query_point), sizeof(Vec3));
                }
                query_point.z += grid_step;
            }
            query_point.y += grid_step;
            query_point.z = bb_min.z;
        }
        query_point.x += grid_step;
        query_point.y = bb_min.y;
        query_point.z = bb_min.z;
    }
}
