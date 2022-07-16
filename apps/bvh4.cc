#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

#include "stl_io.hh"
#include "vec3.hh"

using namespace mp::io;

#ifdef NDEBUG
#define tassert(x) ()
#else
#define tassert(x) \
    if (!(x))      \
    {              \
        throw;     \
    }
#endif

struct BVHNode
{
    BVHNode *L, *R;
    Vec3 aabb_max, aabb_min;
    std::vector<stl::Triangle>::iterator start, end;
    int count() const
    {
        return end - start + 1;
    }
};

inline Vec3 centroid(const stl::Triangle &t)
{
    Vec3 *verts = (Vec3 *)t.verts;
    return (verts[0] + verts[1] + verts[2]) / 3;
}

void recalc_bounds(BVHNode *node, const std::vector<stl::Triangle> &tris)
{
    node->aabb_max = -INFINITY;
    node->aabb_min = INFINITY;
    // tassert(node->start >= 0);
    // tassert(node->start < tris.size());
    // tassert(node->end >= 0);
    // tassert(node->end < tris.size());

    for (auto it = node->start; it < node->end; it++)
    {
        for (int vi = 0; vi < 3; vi++)
        {
            node->aabb_max.max(it->verts[vi]);
            node->aabb_min.min(it->verts[vi]);
        }
    }
}

void subdivide(BVHNode *nodes_pool, BVHNode *root, std::vector<stl::Triangle> &tris, int num_used_nodes)
{
    if (root->count() <= 2)
    {
        return;
    }
    Vec3 dims = root->aabb_max - root->aabb_min;
    int split_axis = 0;
    if (dims.y < dims.x)
    {
        split_axis = 1;
    }
    if (dims.z > dims[split_axis])
    {
        split_axis = 2;
    }
    float split_pos = root->aabb_min[split_axis] + dims[split_axis] * .5;

    auto it = std::partition(root->start, root->end, [=](const stl::Triangle &t)
                             { return centroid(t)[split_axis] < split_pos; });

    if ((it == root->start) || (it == root->end))
    {
        // abort split
        return;
    }

    BVHNode *L = nodes_pool + (num_used_nodes++);
    L->start = root->start;
    L->end = it - 1;
    recalc_bounds(L, tris);
    L->L = L->R = nullptr;

    BVHNode *R = nodes_pool + (num_used_nodes++);
    R->start = it;
    R->end = root->end;
    recalc_bounds(R, tris);
    R->L = R->R = nullptr;

    root->L = L;
    root->R = R;

    subdivide(nodes_pool, L, tris, num_used_nodes);
    subdivide(nodes_pool, R, tris, num_used_nodes);
}

int count(BVHNode *root)
{
    if (root == nullptr)
    {
        return 0;
    }
    return 1 + count(root->L) + count(root->R);
};

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

    BVHNode *root = nodes;
    root->start = tris.begin();
    root->end = tris.end();
    root->L = root->R = nullptr;
    recalc_bounds(root, tris);

    subdivide(nodes, root, tris, 1);
    std::cout << count(root) << std::endl;

    return 0;
}