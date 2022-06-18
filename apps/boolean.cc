#include <chrono>
#include <iostream>
#include <vector>
#include <cmath>

#include "stl_importer.hh"
#include "vec3.hh"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::Simple_cartesian<double> K;
typedef K::FT FT;
typedef K::Ray_3 Ray;
typedef K::Line_3 Line;
typedef K::Point_3 Point;
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
        std::list<Triangle_intersection> intersections;
        tree_1.all_intersections(t2, std::back_inserter(intersections));
        for (auto const &i : intersections)
        {
            auto p = boost::get<Point>(&(i->first));
            if (p)
            {
                std::cout << *p << std::endl;
            }
        }
    }

    return 0;
}
