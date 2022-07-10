#include <vector>
#include <cstdio>
#include <limits>
#include <execution>
#include <iostream>

#include "stl_io.hh"
#include "vec3.hh"
#include "timers.hh"

using namespace mp::io;

struct Bounds
{
    Vec3 max{-std::numeric_limits<float>::infinity()};
    Vec3 min{std::numeric_limits<float>::infinity()};
    void extend(const Vec3 &v)
    {
        max.max(v);
        min.min(v);
    }
};

struct Triangle
{
    Vec3 a, b, c;
    Bounds bounds;
};

struct BooleanBounds
{
    // Avreage bounding box dimensions for triangles
    Vec3 avg_dims{};
    // Bounding box containing all triangles
    Bounds bb{};
    BooleanBounds operator+(const BooleanBounds &other)
    {
        BooleanBounds out;
        out.avg_dims = avg_dims + other.avg_dims;
        Vec3::max(out.bb.max, bb.max, other.bb.max);
        Vec3::min(out.bb.min, bb.min, other.bb.min);
        return out;
    }
};

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        puts("Usage: boolean4 a.stl b.stl");
        return 1;
    }

    const char *filepath_1 = argv[1];
    const char *filepath_2 = argv[2];

    std::vector<stl::Triangle> tri_soup;
    stl::read_stl(filepath_1, tri_soup);
    stl::read_stl(filepath_2, tri_soup);

    size_t tris_num = tri_soup.size();
    auto map_func = [&](const stl::Triangle &t)
    {
        Vec3 *verts = (Vec3 *)t.verts;
        Bounds bb;
        bb.max = -std::numeric_limits<float>::infinity();
        bb.min = std::numeric_limits<float>::infinity();
        for (int i = 0; i < 3; i++)
        {
            bb.extend(verts[i]);
        }
        return BooleanBounds{(bb.max - bb.min) / tris_num, bb};
    };
    Timer timer;
    BooleanBounds boolean_bounds = std::transform_reduce(std::execution::par,
                                                         tri_soup.cbegin(),
                                                         tri_soup.cend(),
                                                         BooleanBounds(),
                                                         std::plus{},
                                                         map_func);
    timer.tock("Calculating avreage bounding box dimensions for triangles.");

    printf("Avg. BB <(%f, %f, %f)>\n", boolean_bounds.avg_dims.x, boolean_bounds.avg_dims.y, boolean_bounds.avg_dims.z);

    std::cout << "Avg. Triangle BB = " << boolean_bounds.avg_dims << std::endl;
    std::cout << "Mesh Bounding Box: Max(" << boolean_bounds.bb.max << "), Min(" << boolean_bounds.bb.min << ")" << std::endl;
    return 0;
}
