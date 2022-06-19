#include <cstdio>
#include <vector>
#include <iostream>

#include "stl_io.hh"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::Simple_cartesian<double> K;

typedef K::Point_3 Point3;
typedef K::Triangle_3 Triangle;

typedef std::vector<Triangle>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<K, Iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;

static std::vector<Triangle> load_stl(const char *filepath)
{
    std::vector<mp::io::stl::Triangle> tris;
    mp::io::stl::read_stl(filepath, tris);

    std::vector<Triangle> out;
    for (auto const &t : tris)
    {
        Point3 p1(t.v1[0], t.v1[1], t.v1[2]);
        Point3 p2(t.v2[0], t.v2[1], t.v2[2]);
        Point3 p3(t.v3[0], t.v3[1], t.v3[2]);

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

    auto tri_soup_1 = load_stl(argv[1]);
    Tree tree_1(tri_soup_1.begin(), tri_soup_1.end());

    auto tri_soup_2 = load_stl(argv[2]);
}
