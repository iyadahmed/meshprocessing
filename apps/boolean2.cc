#include <cstdio>
#include <vector>
#include <iostream>

#include "stl_io.hh"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel_with_kth_root.h>
#include <CGAL/Triangulation_2.h>

typedef CGAL::Simple_cartesian<double> TreeK;

typedef TreeK::Point_3 Point;
typedef TreeK::Triangle_3 Triangle;
typedef TreeK::Segment_3 Segment;

typedef std::vector<Triangle>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<TreeK, Iterator> Primitive;
typedef CGAL::AABB_traits<TreeK, Primitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;

typedef Tree::Primitive_id Primitive_id;
typedef boost::optional<Tree::Intersection_and_primitive_id<Triangle>::Type> Triangle_intersection;

typedef CGAL::Exact_predicates_inexact_constructions_kernel TriK;
typedef CGAL::Triangulation_2<TriK> Triangulation;
typedef Triangulation::Point TriPoint;

static std::vector<Triangle> load_stl(const char *filepath)
{
    std::vector<mp::io::stl::Triangle> tris;
    mp::io::stl::read_stl(filepath, tris);

    std::vector<Triangle> out;
    for (auto const &t : tris)
    {
        Point p1(t.v1[0], t.v1[1], t.v1[2]);
        Point p2(t.v2[0], t.v2[1], t.v2[2]);
        Point p3(t.v3[0], t.v3[1], t.v3[2]);

        out.push_back({p1, p2, p3});
    }

    return out;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        puts("Usage: boolean a.stl b.stl");
        return 1;
    }

    std::cout << "CGAL Version: " << CGAL_VERSION_STR << std::endl;

    auto tri_soup_1 = load_stl(argv[1]);
    Tree tree_1(tri_soup_1.begin(), tri_soup_1.end());

    auto tri_soup_2 = load_stl(argv[2]);

    for (auto const &tri : tri_soup_2)
    {
        std::vector<Triangle_intersection> intersections;
        tree_1.all_intersections(tri, std::back_inserter(intersections));
        std::cout << "Number of intersections = " << intersections.size() << std::endl;
        for (auto const &ti : intersections)
        {
            if (auto p = boost::get<Point>(&(ti->first)))
            {
                std::cout << "Intersected Point: " << *p << std::endl;
            }
            else if (auto s = boost::get<Segment>(&(ti->first)))
            {
                std::cout << "Intersected Segment: " << *s << std::endl;
            }
            else if (auto t = boost::get<Triangle>(&(ti->first)))
            {
                std::cout << "Intersected Triangle: " << *t << std::endl;
            }
            else if (auto points = boost::get<std::vector<Point>>(&(ti->first)))
            {
                std::cout << "Intersected Polygon: " << std::endl;
                for (auto const &p : *points)
                {
                    std::cout << p << std::endl;
                }
            }
        }
    }
}
