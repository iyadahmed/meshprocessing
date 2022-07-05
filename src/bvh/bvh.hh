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
};

BVHNode *build_bvh(std::vector<BVHTriangle> &tris);
