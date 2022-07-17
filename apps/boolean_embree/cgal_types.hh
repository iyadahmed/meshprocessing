// Fixes CGAL assert when running debug build under valgrind
#define CGAL_DISABLE_ROUNDING_MATH_CHECK

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
// #include <CGAL/Exact_predicates_exact_constructions_kernel.h>
// #include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

#include "stl_io.hh"
#include "vec3.hh"

// typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Simple_cartesian<float> K;
typedef K::Triangle_3 Triangle;
typedef K::Segment_3 Segment;
typedef K::Point_3 Point;

struct TriangulationPointInfo
{
    Point point_3d;
};

typedef CGAL::Triangulation_vertex_base_with_info_2<TriangulationPointInfo, K> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb> Tds;
typedef CGAL::Triangulation_2<K, Tds> Triangulation;

using namespace mp::io;

inline Triangle to_cgal_triangle(const stl::Triangle &t)
{
    return {
        {t.v1[0], t.v1[1], t.v1[2]},
        {t.v2[0], t.v2[1], t.v2[2]},
        {t.v3[0], t.v3[1], t.v3[2]},
    };
}

inline Triangle to_cgal_triangle(const Vec3 &a, const Vec3 &b, const Vec3 &c)
{
    return {
        {a.x, a.y, a.z},
        {b.x, b.y, b.z},
        {c.x, c.y, c.z},
    };
}

inline Segment to_cgal_segment(const Vec3 &a, const Vec3 &b)
{
    return {
        {a.x, a.y, a.z},
        {b.x, b.y, b.z},
    };
}