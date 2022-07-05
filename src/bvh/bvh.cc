#include "bvh.hh"
#include <assert.h>

static inline void intersect_ray_tri(BVHRay &ray, const BVHTriangle &tri)
{
    const Vec3 edge1 = tri.vertex1 - tri.vertex0;
    const Vec3 edge2 = tri.vertex2 - tri.vertex0;
    const Vec3 h = ray.direction.cross(edge2);
    const float a = edge1.dot(h);
    if (a > -0.0001f && a < 0.0001f)
        return; // ray parallel to triangle
    const float f = 1 / a;
    const Vec3 s = ray.origin - tri.vertex0;
    const float u = f * s.dot(h);
    if (u < 0 || u > 1)
        return;
    const Vec3 q = s.cross(edge1);
    const float v = f * ray.direction.dot(q);
    if (v < 0 || u + v > 1)
        return;
    const float t = f * edge2.dot(q);
    if (t > 0.0001f)
        ray.t = std::min(ray.t, t);
}

static inline bool intersect_ray_aabb(const BVHRay &ray, const Vec3 &bmin, const Vec3 &bmax)
{
    float tx1 = (bmin.x - ray.origin.x) / ray.direction.x, tx2 = (bmax.x - ray.origin.x) / ray.direction.x;
    float tmin = std::min(tx1, tx2), tmax = std::max(tx1, tx2);
    float ty1 = (bmin.y - ray.origin.y) / ray.direction.y, ty2 = (bmax.y - ray.origin.y) / ray.direction.y;
    tmin = std::max(tmin, std::min(ty1, ty2)), tmax = std::min(tmax, std::max(ty1, ty2));
    float tz1 = (bmin.z - ray.origin.z) / ray.direction.z, tz2 = (bmax.z - ray.origin.z) / ray.direction.z;
    tmin = std::max(tmin, std::min(tz1, tz2)), tmax = std::min(tmax, std::max(tz1, tz2));
    return tmax >= tmin && tmin < ray.t && tmax > 0;
}

void BVH::intersect_ray(BVHRay &ray, int node_index) const
{
    BVHNode &node = m_nodes[node_index];
    if (!intersect_ray_aabb(ray, node.aabb_min, node.aabb_max))
    {
        return;
    }
    if (node.is_leaf())
    {
        for (int i = 0; i < node.triangle_count; i++)
        {
            intersect_ray_tri(ray, m_tris[m_tris_indices[node.first_triangle_index + i]]);
        }
    }
    else
    {
        intersect_ray(ray, node.left_child);
        intersect_ray(ray, node.left_child + 1);
    }
}

BVH::BVH(int tris_num)
{
    m_tris = new BVHTriangle[tris_num]{};
    m_tris_indices = new int[tris_num];
    for (int i = 0; i < tris_num; i++)
    {
        m_tris_indices[i] = i;
    }
    m_tris_num = tris_num;
    m_nodes = nullptr;
    m_used_nodes_num = 1;
    // Defer build to build method, so that user has a chance to set triangles
    // TODO: maybe refactor that?
}

void BVH::update_node_bounds(int node_index)
{
    BVHNode &node = m_nodes[node_index];
    node.aabb_max = Vec3(-1e30);
    node.aabb_max = Vec3(1e30);

    for (int first = node.first_triangle_index, i = 0; i < node.triangle_count; i++)
    {
        int tri_index = m_tris_indices[first + i];
        const BVHTriangle &leaf_tri = m_tris[tri_index];
        node.aabb_max.max(leaf_tri.vertex0);
        node.aabb_max.max(leaf_tri.vertex1);
        node.aabb_max.max(leaf_tri.vertex2);

        node.aabb_min.min(leaf_tri.vertex0);
        node.aabb_min.min(leaf_tri.vertex1);
        node.aabb_min.min(leaf_tri.vertex2);
    }
}

void BVH::subdivide(int node_index)
{
    // Terminate recursion
    BVHNode &node = m_nodes[node_index];
    if (node.triangle_count <= 2)
    {
        return;
    }
    // Split along longest axis
    Vec3 extent = node.aabb_max - node.aabb_min;
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
    // In-place partition
    int i = node.first_triangle_index;
    int j = i + node.triangle_count - 1;
    while (i <= j)
    {
        if (m_tris[m_tris_indices[i]].centroid[axis] < split_pos)
        {
            i++;
        }
        else
        {
            std::swap(m_tris_indices[i], m_tris_indices[j--]);
        }
    }
    // Abort split if one of the sides is empty
    int left_count = i - node.first_triangle_index;
    if (left_count == 0 || left_count == node.triangle_count)
    {
        return;
    }
    // Create child nodes
    int left_child_index = m_used_nodes_num++;
    int right_child_index = m_used_nodes_num++;
    node.left_child = left_child_index;

    m_nodes[left_child_index].first_triangle_index = node.first_triangle_index;
    m_nodes[left_child_index].triangle_count = left_count;
    m_nodes[right_child_index].first_triangle_index = i;
    m_nodes[right_child_index].triangle_count = node.triangle_count - left_count;

    node.triangle_count = 0;

    update_node_bounds(left_child_index);
    update_node_bounds(right_child_index);
    subdivide(left_child_index);
    subdivide(right_child_index);
}

void BVH::build()
{
    // TODO: allow rebuilding the BVH
    if (m_nodes)
    {
        puts("Warning: BVH already built, doing nothing");
        return;
    }
    for (int i = 0; i < m_tris_num; i++)
    {
        m_tris[i].centroid = .3333f * (m_tris[i].vertex0 + m_tris[i].vertex1 + m_tris[i].vertex2);
    }
    m_nodes = new BVHNode[2 * m_tris_num - 1];
    BVHNode &root = m_nodes[0];
    root.right_child = root.left_child = 0;
    root.first_triangle_index = 0;
    root.triangle_count = m_tris_num;
    update_node_bounds(0);
    subdivide(0);
}

BVHTriangle &BVH::triangle(int index) const
{
    return m_tris[index];
}

BVH::~BVH()
{
    delete[] m_tris;
    delete[] m_tris_indices;
    if (m_nodes)
    {
        delete[] m_nodes;
    }
}
