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
    if (argc != 2)
    {
        puts("Usage: boolean input.stl");
        return 1;
    }

    // Load mesh
    std::vector<Triangle> mesh;
    read_stl(argv[1], mesh);

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

    std::ofstream file("filtered_points.pts", std::ios::binary);

    Vec3 query_point = bb_min;
    Vec3 bb_dims = bb_max - bb_min;
    float voxel_size = 5.0f;
    int num_x = static_cast<int>(bb_dims.x / voxel_size);
    int num_y = static_cast<int>(bb_dims.y / voxel_size);
    int num_z = static_cast<int>(bb_dims.z / voxel_size);
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
                query_point.z += voxel_size;
            }
            query_point.y += voxel_size;
            query_point.z = bb_min.z;
        }
        query_point.x += voxel_size;
        query_point.y = bb_min.y;
        query_point.z = bb_min.z;
    }
}
