// Fixes CGAL assert when running debug build under valgrind
#define CGAL_DISABLE_ROUNDING_MATH_CHECK

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
// #include <CGAL/Exact_predicates_exact_constructions_kernel.h>
// #include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

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