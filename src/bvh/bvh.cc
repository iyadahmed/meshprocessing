#include "bvh.hh"

static void intersect_ray_tri(BVHRay &ray, const BVHTriangle &tri)
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

static bool intersect_ray_aabb(const BVHRay &ray, const Vec3 &bmin, const Vec3 &bmax)
{
    float tx1 = (bmin.x - ray.origin.x) / ray.direction.x, tx2 = (bmax.x - ray.origin.x) / ray.direction.x;
    float tmin = std::min(tx1, tx2), tmax = std::max(tx1, tx2);
    float ty1 = (bmin.y - ray.origin.y) / ray.direction.y, ty2 = (bmax.y - ray.origin.y) / ray.direction.y;
    tmin = std::max(tmin, std::min(ty1, ty2)), tmax = std::min(tmax, std::max(ty1, ty2));
    float tz1 = (bmin.z - ray.origin.z) / ray.direction.z, tz2 = (bmax.z - ray.origin.z) / ray.direction.z;
    tmin = std::max(tmin, std::min(tz1, tz2)), tmax = std::min(tmax, std::max(tz1, tz2));
    return tmax >= tmin && tmin < ray.t && tmax > 0;
}

static void update_node_bounds(BVHNode &node, const std::vector<BVHTriangle> &tris)
{
    node.aabb_max = Vec3(-1e30);
    node.aabb_max = Vec3(1e30);

    for (uint32_t first = node.first_triangle_index, i = 0; i < node.triangle_count; i++)
    {
        const BVHTriangle &leaf_tri = tris[first + i];
        node.aabb_max.max(leaf_tri.vertex0);
        node.aabb_max.max(leaf_tri.vertex1);
        node.aabb_max.max(leaf_tri.vertex2);

        node.aabb_min.min(leaf_tri.vertex0);
        node.aabb_min.min(leaf_tri.vertex1);
        node.aabb_min.min(leaf_tri.vertex2);
    }
}

static void subdivide(BVHNode *nodes, BVHNode &node, const std::vector<BVHTriangle> &tris, std::vector<uint32_t> &tris_indices, int used_nodes_num = 0)
{
    // Terminate recursion
    if (node.triangle_count < 2)
    {
        return;
    }

    // Determine split axis and position
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
    float split_position = node.aabb_min[axis] + extent[axis] * .5f;

    // In-place parition
    int i = node.first_triangle_index;
    int j = i + node.triangle_count - 1;
    while (i < j)
    {
        if (tris[tris_indices[i]].centroid[axis] < split_position)
        {
            i++;
        }
        else
        {
            std::swap(tris_indices[i], tris_indices[j--]);
        }
    }

    // Abort split if one of the sides is empty
    int left_count = i - node.first_triangle_index;
    if (left_count == 0 || left_count == node.triangle_count)
    {
        return;
    }

    // Create child nodes
    int left_child_index = used_nodes_num++;
    int right_child_index = used_nodes_num++;

    node.left_child = left_child_index;

    nodes[left_child_index].first_triangle_index = node.first_triangle_index;
    nodes[left_child_index].triangle_count = left_count;

    nodes[right_child_index].first_triangle_index = i;
    nodes[right_child_index].triangle_count = node.triangle_count - left_count;
    node.triangle_count = 0;

    update_node_bounds(nodes[left_child_index], tris);
    update_node_bounds(nodes[right_child_index], tris);

    // Recurse
    subdivide(nodes, nodes[left_child_index], tris, tris_indices, used_nodes_num);
    subdivide(nodes, nodes[right_child_index], tris, tris_indices, used_nodes_num);
}

// TODO: refactor into BVH class and get rid of unnecessary bloat
BVHNode *build_bvh(std::vector<BVHTriangle> &tris)
{
    for (auto &t : tris)
    {
        t.centroid = (t.vertex0 + t.vertex1 + t.vertex2) * 0.333333f;
    }

    BVHNode *nodes = new BVHNode[tris.size() - 1];

    BVHNode &root = nodes[0];
    root.right_child = root.left_child = 0;
    root.first_triangle_index = 0;
    root.triangle_count = tris.size();

    std::vector<uint32_t> tris_indices;
    tris_indices.reserve(tris.size());
    for (int i = 0; i < tris.size(); i++)
    {
        tris_indices.push_back(i);
    }

    update_node_bounds(root, tris);
    subdivide(nodes, root, tris, tris_indices);
    return nodes;
}

static void intersect_ray_bvh(BVHNode *nodes, BVHRay &ray, BVHNode &node, const std::vector<BVHTriangle> &tris, std::vector<uint32_t> &tris_indices)
{
    if (!intersect_ray_aabb(ray, node.aabb_min, node.aabb_max))
        return;
    if (node.is_leaf())
    {
        for (uint i = 0; i < node.triangle_count; i++)
            intersect_ray_tri(ray, tris[tris_indices[node.first_triangle_index + i]]);
    }
    else
    {
        intersect_ray_bvh(nodes, ray, nodes[node.left_child], tris, tris_indices);
        intersect_ray_bvh(nodes, ray, nodes[node.left_child], tris, tris_indices);
    }
}

// TODO: refactor
BVH::BVH(std::vector<BVHTriangle> &tris)
{

    m_tris = tris;

    // Update centroids
    for (auto &t : tris)
    {
        t.centroid = (t.vertex0 + t.vertex1 + t.vertex2) * 0.333333f;
    }

    m_nodes = new BVHNode[tris.size() - 1];
    BVHNode &root = m_nodes[0];
    root.right_child = root.left_child = 0;
    root.first_triangle_index = 0;
    root.triangle_count = tris.size();

    m_tris_indices.reserve(tris.size());
    for (int i = 0; i < tris.size(); i++)
    {
        m_tris_indices.push_back(i);
    }

    update_node_bounds(root, tris);
    subdivide(m_nodes, root, tris, m_tris_indices);
}

BVH::~BVH()
{
    delete[] m_nodes;
}

void BVH::ray_intersection(BVHRay &ray)
{
    intersect_ray_bvh(m_nodes, ray, m_nodes[0], m_tris, m_tris_indices);
}
