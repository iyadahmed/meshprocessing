#include <vector>

#include "vec3.hh"

// struct float3
// {
//     float x, y, z;
// };

struct BVHTriangle
{
    Vec3 vertex0, vertex1, vertex2, centroid;
};

struct BVHRay
{
    Vec3 origin, direction;
    float t = 1e30f;
};
struct BVHNode
{
    Vec3 aabb_min, aabb_max;
    int left_child, right_child;
    int first_triangle_index, triangle_count;
    bool is_leaf() const { return triangle_count > 0; };
};

class BVH
{
private:
    BVHNode *m_nodes;
    int m_used_nodes_num;
    // TODO: share triangles between BVH and the outside world (e.g. animation system)
    BVHTriangle *m_tris;
    int *m_tris_indices;
    int m_tris_num;

    void update_node_bounds(int node_index);
    void subdivide(int node_index);

public:
    BVH(int tris_num);
    ~BVH();

    void build();
    BVHTriangle &triangle(int index) const;
    void intersect_ray(BVHRay &ray, int node_index = 0) const;
};
