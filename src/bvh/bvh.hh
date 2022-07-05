#include <vector>

#include "vec3.hh"

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
    uint32_t left_child, right_child;
    uint32_t first_triangle_index, triangle_count;
    bool is_leaf() const { return triangle_count > 0; };
};

class BVH
{
private:
    BVHNode *m_nodes;
    std::vector<uint32_t> m_tris_indices;
    std::vector<BVHTriangle> m_tris;

public:
    BVH(std::vector<BVHTriangle> &tris);
    ~BVH();

    void ray_intersection(BVHRay &ray);
};
