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

    for (auto const &t : tri_soup_2)
    {
        std::vector<Triangle_intersection> intersections;
        std::vector<Primitive_id> primitives;
        tree_1.all_intersected_primitives(t, std::back_inserter(primitives));
        // tree_1.all_intersections(t, std::back_inserter(intersections));
        std::cout << "Number of intersections = " << primitives.size() << std::endl;

        for (auto const &other_t_id : primitives)
        {
            Segment t1_s1(t.vertex(0), t.vertex(1));
            Segment t1_s2(t.vertex(1), t.vertex(2));
            Segment t1_s3(t.vertex(2), t.vertex(0));

            Segment tri_segments[] = {t1_s1, t1_s2, t1_s3};

            for (int i = 0; i < 3; i++)
            {
                auto s = tri_segments[i];
                auto intersection_result = CGAL::intersection(s, *other_t_id);

                if (auto p = boost::get<Point>(&*intersection_result))
                {
                    std::cout << "Intersected Point: " << *p << std::endl;
                }
                else if (auto s = boost::get<Segment>(&*intersection_result))
                {
                    std::cout << "Intersected Segment: " << *s << std::endl;
                }
            }
        }

        // for (auto const &ti : intersections)
        // {
        //     auto p = boost::get<Point3>(&(ti->first));
        //     if (p)
        //     {
        //         std::cout << *p << std::endl;
        //     }
        // }
    }
}
