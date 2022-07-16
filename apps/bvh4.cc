#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>

#include "stl_io.hh"

using namespace mp::io;

#include <cmath>

struct float3
{
    float x, y, z;
    float3(float value = 0.0f)
    {
        x = y = z = value;
    }
    float3(float x, float y, float z) : x(x), y(y), z(z)
    {
    }
    float &operator[](int index)
    {
        return reinterpret_cast<float *>(this)[index];
    }
    void max(const float3 &other)
    {
        x = std::max(x, other.x);
        y = std::max(y, other.y);
        z = std::max(z, other.z);
    }
    void min(const float3 &other)
    {
        x = std::min(x, other.x);
        y = std::min(y, other.y);
        z = std::min(z, other.z);
    }
    float3 operator-(const float3 &other) const
    {
        return {x - other.x, y - other.y, z - other.z};
    }
    float3 operator+(const float3 &other) const
    {
        return {x + other.x, y + other.y, z + other.z};
    }
    float3 operator*(float value)
    {
        return {x * value, y * value, z * value};
    }
    float3 cross(const float3 &other) const
    {
        return {y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x};
    }
    float dot(const float3 &other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }
    void normalize()
    {
        float l = std::sqrt(x * x + y * y + z * z);
        x /= l;
        y /= l;
        z /= l;
    }
};

struct BVHNode
{
    BVHNode *L, *R;
    float3 aabb_max, aabb_min;
    int start, end;
    int count() const
    {
        return end - start + 1;
    }
};

void print_float3(const float3 &v)
{
    printf("(%f, %f, %f)", v.x, v.y, v.z);
}

float3 centroid(const stl::Triangle &t)
{
    float3 out{0.0f, 0.0f, 0.0f};
    for (int i = 0; i < 3; i++)
    {
        out.x += (t.verts[i][0] / 3);
        out.y += (t.verts[i][1] / 3);
        out.z += (t.verts[i][2] / 3);
    }
    return out;
}

void recalc_bounds(BVHNode *node, const std::vector<stl::Triangle> &tris)
{
    node->aabb_max = {-INFINITY, -INFINITY, -INFINITY};
    node->aabb_min = {INFINITY, INFINITY, INFINITY};
    if (node->start < 0)
    {
        throw;
    }
    if (node->start >= tris.size())
    {
        throw;
    }
    if (node->end < 0)
    {
        throw;
    }
    if (node->end >= tris.size())
    {
        throw;
    }
    for (int i = node->start; i < node->end; i++)
    {
        for (int vi = 0; vi < 3; vi++)
        {
            for (int ci = 0; ci < 3; ci++)
            {
                node->aabb_max[ci] = fmaxf(node->aabb_max[ci], tris[i].verts[vi][ci]);
                node->aabb_min[ci] = fminf(node->aabb_max[ci], tris[i].verts[vi][ci]);
            }
        }
    }
}

void subdivide(BVHNode *nodes_pool, BVHNode *root, std::vector<stl::Triangle> &tris, int num_used_nodes)
{
    if (root->count() <= 2)
    {
        return;
    }
    float3 dims = root->aabb_max - root->aabb_min;
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

    int left_parition_count = root->start;
    int j = root->end;
    while (left_parition_count <= j)
    {
        if (centroid(tris[left_parition_count])[split_axis] < split_pos)
        {
            left_parition_count++;
        }
        else
        {
            std::swap(tris[left_parition_count], tris[j--]);
        }
    }

    if ((left_parition_count == root->count()) || (left_parition_count == 0))
    {
        // abort split
        return;
    }

    BVHNode *L = nodes_pool + (num_used_nodes++);
    L->start = root->start;
    L->end = root->start + left_parition_count - 1;
    recalc_bounds(L, tris);
    L->L = L->R = nullptr;

    BVHNode *R = nodes_pool + (num_used_nodes++);
    R->start = L->end + 1;
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
    root->start = 0;
    root->end = tris.size() - 1;
    root->L = root->R = nullptr;
    recalc_bounds(root, tris);

    subdivide(nodes, root, tris, 1);
    std::cout << count(root) << std::endl;

    return 0;
}