#include <vector>

#include "stl_io.hh"
#include "vec3.hh"

using namespace mp::io;

struct BVHNode
{
    Vec3 aabb_max, aabb_min;
    int start_primitive_index, end_primitive_index;
    BVHNode *right_child, *left_child;
};

Vec3 centroid(const stl::Triangle &t)
{
    Vec3 *verts = (Vec3 *)t.verts;
    return (verts[0] + verts[1] + verts[2]) / 3;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        puts("Usage: bvh4 input.stl");
        return 1;
    }

    std::vector<stl::Triangle> tris;
    stl::read_stl(argv[1], tris);

    if (tris.size() == 0)
    {
        puts("Empty mesh");
        return 0;
    }

    BVHNode *nodes = new BVHNode[2 * tris.size() - 1];
    BVHNode &root_node = nodes[0];
    Vec3 aabb_max(-INFINITY), aabb_min(INFINITY);
    for (const auto &t : tris)
    {
        for (int i = 0; i < 3; i++)
        {
            aabb_max.max(t.verts[i]);
            aabb_min.min(t.verts[i]);
        }
    }
    root_node.aabb_max = aabb_max;
    root_node.aabb_min = aabb_min;
    root_node.start_primitive_index = 0;
    root_node.end_primitive_index = tris.size() - 1;
    root_node.left_child = nullptr;
    root_node.right_child = nullptr;

    Vec3 dims = aabb_max - aabb_min;
    int split_axis = 0;
    if (dims.y > dims.x)
    {
        split_axis = 1;
    }
    if (dims.z > dims[split_axis])
    {
        split_axis = 2;
    }

    float split_position = aabb_min[split_axis] + dims[split_axis] * .5;

    int i = root_node.start_primitive_index;
    int j = root_node.end_primitive_index;
    while (i <= j)
    {
        if (centroid(tris[i])[split_axis] < split_position)
        {
            i++;
        }
        else
        {
            std::swap(tris[i], tris[j]);
            j--;
        }
    }

    return 0;
}