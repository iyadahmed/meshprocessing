// #include <vector>

#include <cmath>

// #include "vec3.hh"

// // struct float3
// // {
// //     float x, y, z;
// //     float3 cross(const float3 &other) const
// //     {
// //         float3 out;
// //         out.x = y * other.z - z * other.y;
// //         out.y = z * other.x - x * other.z;
// //         out.z = x * other.y - y * other.x;
// //         return out;
// //     }
// //     Vec3 operator-(const Vec3 &other) const
// //     {
// //         return Vec3(x - other.x, y - other.y, z - other.z);
// //     }
// // };

// // typedef float3 Vec3;

// struct BVHTriangle
// {
//     Vec3 vertex0, vertex1, vertex2, centroid;
// };

// struct BVHRay
// {
//     Vec3 origin, direction;
//     float t = 1e30f;
// };
// struct BVHNode
// {
//     Vec3 aabb_min, aabb_max;
//     int left_child, right_child;
//     int first_triangle_index, triangle_count;
//     bool is_leaf() const { return triangle_count > 0; };
// };

// class BVH
// {
// private:
//     BVHNode *m_nodes;
//     int m_used_nodes_num;
//     // TODO: share triangles between BVH and the outside world (e.g. animation system)
//     BVHTriangle *m_tris;
//     int *m_tris_indices;
//     int m_tris_num;

//     void update_node_bounds(int node_index);
//     void subdivide(int node_index);

// public:
//     BVH(int tris_num);
//     ~BVH();

//     void build();
//     BVHTriangle &triangle(int index) const;
//     void intersect_ray(BVHRay &ray, int node_index = 0) const;
// };

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

struct BVHTriangle
{
    float3 v0, v1, v2, centroid;
};

struct BVHRay
{
    float3 O, D;
};

struct BVHNode
{
    float3 aabb_min, aabb_max;
    int left_child, right_child;
    int first_tri_index, tri_count;
    bool is_leaf() const { return tri_count; }
};

class BVH
{
private:
    BVHNode *nodes_;
    BVHTriangle *tris_;
    int tris_num_;

public:
    BVH(int tris_num) : tris_num_(tris_num)
    {
        nodes_ = new BVHNode[tris_num * 2]{};
        tris_ = new BVHTriangle[tris_num]{};
    }
    ~BVH()
    {
        delete[] nodes_;
        delete[] tris_;
    }
    int tris_num() const
    {
        return tris_num_;
    }
    BVHTriangle &triangle(int index)
    {
        return tris_[index];
    }
    void build()
    {
        // Calculate triangle centroids
        for (int i = 0; i < tris_num_; i++)
        {
            BVHTriangle &t = tris_[i];
            t.centroid = (t.v0 + t.v1 + t.v2) * .3333333f;
        }

        BVHNode &root = nodes_[0];
        root.first_tri_index = root.left_child = root.right_child = 0;
        root.tri_count = tris_num_;
    }
};

#include <assert.h>
void TEST()
{
    BVH bvh(100);
    assert(bvh.tris_num() == 100);
}
