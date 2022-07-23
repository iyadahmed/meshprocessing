#pragma once

#include <algorithm>
#include <cmath>

#include "vec3.hh"
#include "../common.hh"


struct BVHTriangle {
  Vec3 a, b, c;
  Vec3 &operator[](size_t i)
  {
    return reinterpret_cast<Vec3 *>(&(*this))[i];
  }
  Vec3 centroid() const
  {
    return (a + b + c) / 3;
  };
};

struct BVHRay {
  Vec3 O, D;
  float t = INFINITY;
};

struct BVHNode {
  BVHNode *L, *R;
  Vec3 aabb_max, aabb_min;
  std::vector<BVHTriangle>::iterator start, end;
  int count() const { return end - start + 1; }
};

void intersect_ray_tri(BVHRay &ray, const BVHTriangle &tri) {
  const Vec3 edge1 = tri.b - tri.a;
  const Vec3 edge2 = tri.c - tri.a;
  const Vec3 h = cross(ray.D, edge2);
  const float a = dot(edge1, h);
  if (a > -0.0001f && a < 0.0001f)
    return; // ray parallel to triangle
  const float f = 1 / a;
  const Vec3 s = ray.O - tri.a;
  const float u = f * dot(s, h);
  if (u < 0 || u > 1)
    return;
  const Vec3 q = cross(s, edge1);
  const float v = f * dot(ray.D, q);
  if (v < 0 || u + v > 1)
    return;
  const float t = f * dot(edge2, q);
  if (t > 0.0001f)
    ray.t = std::min(ray.t, t);
}

bool intersect_ray_aabb(const BVHRay &ray, const Vec3 &bmin, const Vec3 &bmax) {
  float tx1 = (bmin.x - ray.O.x) / ray.D.x, tx2 = (bmax.x - ray.O.x) / ray.D.x;
  float tmin = std::min(tx1, tx2), tmax = std::max(tx1, tx2);
  float ty1 = (bmin.y - ray.O.y) / ray.D.y, ty2 = (bmax.y - ray.O.y) / ray.D.y;
  tmin = std::max(tmin, std::min(ty1, ty2)),
  tmax = std::min(tmax, std::max(ty1, ty2));
  float tz1 = (bmin.z - ray.O.z) / ray.D.z, tz2 = (bmax.z - ray.O.z) / ray.D.z;
  tmin = std::max(tmin, std::min(tz1, tz2)),
  tmax = std::min(tmax, std::max(tz1, tz2));
  return tmax >= tmin && tmin < ray.t && tmax > 0;
}

class BVH {
private:
  BVHNode *nodes_;

  void recalc_bounds(BVHNode *node, const std::vector<BVHTriangle> &tris) {
    node->aabb_max = -INFINITY;
    node->aabb_min = INFINITY;
    tassert(node->start >= tris.begin());
    tassert(node->start <= tris.end());
    tassert(node->end >= tris.begin());
    tassert(node->end <= tris.end());

    for (auto it = node->start; it < node->end; it++) {
      for (int vi = 0; vi < 3; vi++) {
        node->aabb_max.max((*it)[vi]);
        node->aabb_min.min((*it)[vi]);
      }
    }
  }

  void subdivide(BVHNode *nodes_pool, BVHNode *root,
                 std::vector<BVHTriangle> &tris, int num_used_nodes) {
    if (root->count() <= 2) {
      return;
    }
    Vec3 dims = root->aabb_max - root->aabb_min;
    int split_axis = 0;
    if (dims.y < dims.x) {
      split_axis = 1;
    }
    if (dims.z > dims[split_axis]) {
      split_axis = 2;
    }
    float split_pos = root->aabb_min[split_axis] + dims[split_axis] * .5;

    auto it =
        std::partition(root->start, root->end, [=](const BVHTriangle &t) {
          return t.centroid()[split_axis] < split_pos;
        });

    if ((it == root->start) || (it == root->end)) {
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
  int count_(const BVHNode *root) const {
    if (root == nullptr) {
      return 0;
    }
    return 1 + count_(root->L) + count_(root->R);
  };

public:
  BVH(std::vector<BVHTriangle> &tris) {
    if (tris.size() == 0) {
      throw "Empty mesh";
    }
    nodes_ = new BVHNode[2 * tris.size() - 1];

    BVHNode *root = nodes_;
    root->start = tris.begin();
    root->end = tris.end();
    root->L = root->R = nullptr;
    recalc_bounds(root, tris);

    subdivide(nodes_, root, tris, 1);
  }

  int count() const { return count_(nodes_); }

  ~BVH() { delete[] nodes_; }
  // void intersect_ray(BVHRay &ray, int node_index)
  // {
  //     BVHNode &node = nodes_[node_index];
  //     if (!intersect_ray_aabb(ray, node.aabb_min, node.aabb_max))
  //         return;
  //     if (node.is_leaf())
  //     {
  //         for (int i = node.first_tri_index; i < node.tri_count; i++)
  //             intersect_ray_tri(ray, tris_[i]);
  //     }
  //     else
  //     {
  //         intersect_ray(ray, node.left_child_index);
  //         intersect_ray(ray, node.left_child_index + 1);
  //     }
  // }
};
