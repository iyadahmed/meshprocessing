// Fixes CGAL assert when running debug build under valgrind
#define CGAL_DISABLE_ROUNDING_MATH_CHECK

#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_tree.h>
// #include <CGAL/Exact_predicates_exact_constructions_kernel.h>
// #include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

#include "stl_io.hh"
#include "vec3.hh"

// typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Simple_cartesian<double> K;
typedef K::Triangle_3 Triangle;
typedef K::Segment_3 Segment;
typedef K::Point_3 Point;

struct TriangulationPointInfo {
  Point point_3d;
};

typedef CGAL::Triangulation_vertex_base_with_info_2<TriangulationPointInfo, K>
    Vb;
typedef CGAL::Triangulation_data_structure_2<Vb> Tds;
typedef CGAL::Triangulation_2<K, Tds> Triangulation;

using namespace mp::io;

inline Triangle to_cgal_triangle(const stl::Triangle &t) {
  return {
      {t.v1[0], t.v1[1], t.v1[2]},
      {t.v2[0], t.v2[1], t.v2[2]},
      {t.v3[0], t.v3[1], t.v3[2]},
  };
}

inline Triangle to_cgal_triangle(const Vec3 &a, const Vec3 &b, const Vec3 &c) {
  return {
      {a.x, a.y, a.z},
      {b.x, b.y, b.z},
      {c.x, c.y, c.z},
  };
}

inline Segment to_cgal_segment(const Vec3 &a, const Vec3 &b) {
  return {
      {a.x, a.y, a.z},
      {b.x, b.y, b.z},
  };
}

inline bool is_duplicate(const Point &a, const Point &b) {
  return (a - b).squared_length() <= .00001;
}

inline bool has_point(const Triangle &t, const Point &p) {
  return is_duplicate(t.vertex(0), p) || is_duplicate(t.vertex(1), p) ||
         is_duplicate(t.vertex(2), p);
}

inline bool has_shared_point(const Triangle &t1, const Triangle &t2) {
  return has_point(t1, t2.vertex(0)) || has_point(t1, t2.vertex(1)) ||
         has_point(t1, t2.vertex(2));
}

inline bool is_linked_to_segment(const Triangle &t, const Segment &s) {
  return has_point(t, s.vertex(0)) || has_point(t, s.vertex(1));
}