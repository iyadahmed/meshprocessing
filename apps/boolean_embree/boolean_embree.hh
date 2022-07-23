#pragma once

#include <embree3/rtcore.h>
#include <mutex>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>
#include <vector>

#include "cgal_types.hh"
#include "stl_io.hh"
#include "utils.hh"
#include "vec3.hh"

struct BBox {
  Vec3 max, min;
};

namespace BooleanEmbree {
struct Segment {
  Vec3 &a, &b;
  Segment(Vec3 &a_, Vec3 &b_) : a(a_), b(b_) {}
};
struct TriangleSegments {
  Segment a, b, c;
  TriangleSegments(Vec3 &a_, Vec3 &b_, Vec3 &c_)
      : a(a_, b_), b(b_, c_), c(c_, a_) {}
  Segment &operator[](size_t i) {
    return reinterpret_cast<Segment *>(&(*this))[i];
  }
  const Segment &operator[](size_t i) const {
    return reinterpret_cast<const Segment *>(&(*this))[i];
  }
};
struct Triangle {
  Vec3 a, b, c;
  TriangleSegments segments;
  Triangle(Vec3 a_, Vec3 b_, Vec3 c_) : segments(a, b, c) {}
  BBox calc_bbox() const {
    return {Vec3::max(a, Vec3::max(b, c)), Vec3::min(a, Vec3::min(b, c))};
  };
  bool is_inside(Vec3 p, float tolerance) const {
    return ::is_inside(p, a, b, c, tolerance);
  };
};
}; // namespace BooleanEmbree

// Stores input and output data for rtcCollide,
// must be allocated on memory that can be shared between threads,
// like the heap

struct IntersectionData {
  std::mutex mutex;
  std::vector<BooleanEmbree::Triangle> input_tris;

  using Vec3CVector = tbb::concurrent_vector<Vec3>;
  using TriPointsMap = tbb::concurrent_unordered_map<unsigned int, Vec3CVector>;
  // Maps primitive ids to vector of intersection points that lie on
  // the primitive (triangle)
  // NOTE: you can get rid of the map of vectors, by storing intersection points
  // in a linear vector, then after collision is finished, you can sort that
  // vector by primitive id, so that all points that belong to the same triangle
  // are consequtive, make sure to store the primtive id along side each point
  // so you can use it to sort, also make sure to store two copies of the
  // intersection point, 2 intersection points for each two intersecting
  // triangles, a linear scan after that can be done to re-triangulate every
  // trinagle using its original points + intersection points that lie on it,
  //
  // a map of vectors was chosen in the end because it reduced code
  // complexity.
  TriPointsMap output_triangle_points_map;
};

inline Triangulation::Point project_point(const Point &a, const Triangle &t) {
  auto basis_v1 = t.vertex(1) - t.vertex(0);
  auto basis_v2 = t.vertex(2) - t.vertex(0);
  auto x = CGAL::scalar_product(a - CGAL::ORIGIN, basis_v1);
  auto y = CGAL::scalar_product(a - CGAL::ORIGIN, basis_v2);
  return {x, y};
}

std::pair<Triangulation::Point, TriangulationPointInfo>
to_ip(const Point &p, const Triangle &t) {
  TriangulationPointInfo info{p};
  auto p2d = project_point(p, t);
  return {p2d, info};
}

Vec3 to_vec3(Point p) {
  return {(float)CGAL::to_double(p.x()), (float)CGAL::to_double(p.y()),
          (float)CGAL::to_double(p.z())};
}

#define TOL .00001f
inline void collide_func(void *user_data_ptr, RTCCollision *collisions,
                         unsigned int num_collisions) {
  if (num_collisions == 0)
    return;

  IntersectionData *data = (IntersectionData *)user_data_ptr;
  for (size_t i = 0; i < num_collisions; i++) {
    const auto &c = collisions[i];
    const unsigned &primID0 = c.primID0;
    const unsigned &geomID0 = c.geomID0;
    const unsigned &geomID1 = c.geomID1;
    const unsigned &primID1 = c.primID1;

    if (primID0 == primID1) {
      continue;
    }

    const auto &t1 = data->input_tris[primID0];
    const auto &t2 = data->input_tris[primID1];
    const auto t1_normal = (t1.b - t1.a).cross(t1.c - t1.a);

    for (int si = 0; si < 3; si++) {
      const auto &s = t1.segments[si];
      auto side1 = plane_side(s.a, t1.a, t1_normal, TOL);
      auto side2 = plane_side(s.b, t1.a, t1_normal, TOL);
      if ((side1 == 0) && (side2 == 0)) {
        // Edge is coplanar with triangle2
        bool a = t2.is_inside(s.a, TOL);
        bool b = t2.is_inside(s.b, TOL);
        if (a && b) {
          data->output_triangle_points_map[primID1].push_back(s.a);
          data->output_triangle_points_map[primID1].push_back(s.b);
          // No need to push for primID0, as both of edge vertices already are included in the orignal triangle
          // and original triangle vertices should be included when triangulating
        } else {
          // TODO: do full 2D edge-triangle intersection and push resulting
          // points possible outcomes: Two points, one point
        }
      } else if (side1 != side2) {
        ray_plane_intersection_unchecked(s.a, (s.b - s.a).normalized(), t1.a, t1_normal);
        // TODO: do ray traingle intersection and push resulting point
      } else {
        // Else case explicitly not handled
        // as intersection is impossible if both segment vertices are on the
        // same side of triangle plane
      }
    }
  }
}

void triangle_bounds_func(const struct RTCBoundsFunctionArguments *args) {
  IntersectionData *data = (IntersectionData *)args->geometryUserPtr;
  const BooleanEmbree::Triangle &t = data->input_tris[args->primID];
  const BBox &bbox = t.calc_bbox();
  args->bounds_o->lower_x = bbox.min[0];
  args->bounds_o->lower_y = bbox.min[1];
  args->bounds_o->lower_z = bbox.min[2];
  args->bounds_o->upper_x = bbox.max[0];
  args->bounds_o->upper_y = bbox.max[1];
  args->bounds_o->upper_z = bbox.max[2];
}
