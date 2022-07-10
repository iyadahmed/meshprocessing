#include <vector>
#include <cstdio>
#include <limits>

#include "stl_io.hh"
#include "vec3.hh"

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

    Vec3 avg_dims(0.0f);
    size_t tris_num = tri_soup.size();
    Vec3 bb_max, bb_min;
    for (const auto &t : tri_soup)
    {
        Vec3 *verts = (Vec3 *)t.verts;
        bb_max = -std::numeric_limits<float>::infinity();
        bb_min = std::numeric_limits<float>::infinity();

        for (int i = 0; i < 3; i++)
        {
            bb_max.max(verts[i]);
            bb_min.min(verts[i]);
        }
        avg_dims += (bb_max - bb_min) / tris_num;
    }

    printf("<(%f, %f, %f)>\n", avg_dims.x, avg_dims.y, avg_dims.z);
    return 0;
}
