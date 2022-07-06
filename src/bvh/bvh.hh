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
        x = fmaxf(x, other.x);
        y = fmaxf(y, other.y);
        z = fmaxf(z, other.z);
    }
    void min(const float3 &other)
    {
        x = fminf(x, other.x);
        y = fminf(y, other.y);
        z = fminf(z, other.z);
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
    int used_nodes_num_;
    void update_node_bounds(int node_index)
    {
        BVHNode &node = nodes_[node_index];
        node.aabb_max = float3(-1e10);
        node.aabb_min = float3(1e30);

        for (int i = node.first_tri_index; i < node.tri_count; i++)
        {
            const BVHTriangle &leaf_tri = tris_[i];
            node.aabb_max.max(leaf_tri.v0);
            node.aabb_max.max(leaf_tri.v1);
            node.aabb_max.max(leaf_tri.v2);

            node.aabb_min.min(leaf_tri.v0);
            node.aabb_min.min(leaf_tri.v1);
            node.aabb_min.min(leaf_tri.v2);
        }
    }
    void subdivide(int node_index)
    {
        BVHNode &node = nodes_[node_index];
        if (node.tri_count <= 2)
        {
            return;
        }
        float3 extent = node.aabb_max - node.aabb_min;

        int axis = 0;
        if (extent.y > extent.x)
        {
            axis = 1;
        }
        if (extent.z > extent[axis])
        {
            axis = 2;
        }
        float split_pos = node.aabb_min[axis] + extent[axis] * .5f;

        // In-place parition
        int i = node.first_tri_index;
        int j = node.tri_count + i - 1;
        while (i <= j)
        {
            if (tris_[i].centroid[axis] < split_pos)
            {
                i++;
            }
            else
            {
                std::swap(tris_[i], tris_[j--]);
            }
        }

        // Abort split if one of the sides is empty
        int left_count = i - node.first_tri_index;
        if (left_count == 0 || left_count == node.tri_count)
        {
            return;
        }

        // Create child nodes
        int left_child_index = used_nodes_num_++;
        int right_child_index = used_nodes_num_++;

        nodes_[left_child_index].first_tri_index = node.first_tri_index;
        nodes_[left_child_index].tri_count = left_count;
        nodes_[right_child_index].first_tri_index = i;
        nodes_[right_child_index].tri_count = node.tri_count - left_count;

        node.first_tri_index = left_child_index;
        node.tri_count = 0;
        update_node_bounds(left_child_index);
        update_node_bounds(right_child_index);

        // Recurse
        subdivide(left_child_index);
        subdivide(right_child_index);
    }

public:
    BVH(int tris_num) : tris_num_(tris_num)
    {
        nodes_ = new BVHNode[tris_num * 2]{};
        tris_ = new BVHTriangle[tris_num]{};
        used_nodes_num_ = 1;
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

        update_node_bounds(0);
        subdivide(0);
    }
};

#include <assert.h>
void TEST()
{
    BVH bvh(100);
    assert(bvh.tris_num() == 100);
}
