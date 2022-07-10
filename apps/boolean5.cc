#include <vector>
#include <cstdio>
#include <limits>
#include <execution>

#include "stl_io.hh"
#include "vec3.hh"
#include "timers.hh"

using namespace mp::io;

struct Bounds
{
    Vec3 max;
    Vec3 min;
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
        Vec3 bb_max = -std::numeric_limits<float>::infinity();
        Vec3 bb_min = std::numeric_limits<float>::infinity();
        for (int i = 0; i < 3; i++)
        {
            bb_max.max(verts[i]);
            bb_min.min(verts[i]);
        }
        return (bb_max - bb_min) / tris_num;
    };
    Timer timer;
    Vec3 avg_dims = std::transform_reduce(std::execution::par, tri_soup.cbegin(), tri_soup.cend(), Vec3(0.0f), std::plus{}, map_func);
    timer.tock("Calculating avreage bounding box dimensions for triangles.");

    printf("<(%f, %f, %f)>\n", avg_dims.x, avg_dims.y, avg_dims.z);
    return 0;
}
