#include <chrono>
#include <iostream>
#include <vector>
#include <cmath>

#include "stl_io.hh"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Delaunay_triangulation_2.h>

// typedef CGAL::Simple_cartesian<double> K;

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Projection_traits_xy_3<K> Gt;
typedef CGAL::Delaunay_triangulation_2<Gt> Delaunay;

typedef K::FT FT;
typedef K::Ray_3 Ray;
typedef K::Line_3 Line;
typedef K::Point_3 Point;
typedef K::Point_2 Point2;
typedef K::Triangle_3 Triangle;
typedef std::list<Triangle>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<K, Iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;
typedef boost::optional<Tree::Intersection_and_primitive_id<Triangle>::Type> Triangle_intersection;

static std::vector<mp::io::stl::Triangle> load_stl(const char *filepath)
{
    std::vector<mp::io::stl::Triangle> tris;
    mp::io::stl::read_stl(filepath, tris);
    return tris;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        puts("Usage: boolean a.stl b.stl");
        return 1;
    }

    std::list<Triangle> mesh_1;
    for (auto const &t : load_stl(argv[1]))
    {
        Point p1(t.v1[0], t.v1[1], t.v1[2]);
        Point p2(t.v2[0], t.v2[1], t.v2[2]);
        Point p3(t.v3[0], t.v3[1], t.v3[2]);

        mesh_1.push_back(Triangle(p1, p2, p3));
    }
    std::list<Triangle> mesh_2;
    for (auto const &t : load_stl(argv[2]))
    {
        Point p1(t.v1[0], t.v1[1], t.v1[2]);
        Point p2(t.v2[0], t.v2[1], t.v2[2]);
        Point p3(t.v3[0], t.v3[1], t.v3[2]);

        mesh_2.push_back(Triangle(p1, p2, p3));
    }

    Tree tree_1(mesh_1.begin(), mesh_1.end());

    for (auto const &t2 : mesh_2)
    {
        auto v1 = t2.vertex(0);
        auto v2 = t2.vertex(1);
        auto v3 = t2.vertex(2);

        auto e1 = v2 - v1;
        auto e2 = v3 - v1;

        std::list<Triangle_intersection> intersections;
        tree_1.all_intersections(t2, std::back_inserter(intersections));

        std::list<Point> intersections_2d;
        for (auto const &i : intersections)
        {
            auto p = boost::get<Point>(&(i->first));
            if (p)
            {
                auto x_2d = p->x() * e1.x() + p->y() * e1.y();
                auto y_2d = p->x() * e2.x() + p->y() * e2.y();
                intersections_2d.push_back({x_2d, y_2d, 0.0});
            }
        }

        Delaunay dt(intersections_2d.begin(), intersections_2d.end());
        for (auto it = dt.finite_faces_begin(); it != dt.finite_faces_end(); it++)
        {
            std::cout << dt.triangle(it) << std::endl;
        }
    }

    return 0;
}
