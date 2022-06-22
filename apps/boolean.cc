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
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

typedef CGAL::Simple_cartesian<double> TreeK;

typedef TreeK::Point_3 Point;
typedef TreeK::Triangle_3 Triangle;
typedef TreeK::Segment_3 Segment;
typedef TreeK::Vector_3 Vector;

typedef std::vector<Triangle>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<TreeK, Iterator> Primitive;
typedef CGAL::AABB_traits<TreeK, Primitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;

typedef Tree::Primitive_id Primitive_id;
typedef boost::optional<Tree::Intersection_and_primitive_id<Triangle>::Type> Triangle_intersection;

typedef CGAL::Exact_predicates_inexact_constructions_kernel TriK;
typedef CGAL::Triangulation_vertex_base_with_info_2<Point, TriK> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb> Tds;

typedef CGAL::Triangulation_2<TriK, Tds> Triangulation;
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

// Computes new triangles from intersection points of a triangle with a Tree
static std::vector<Point> triangulation_from_intersection(const Tree &tree, const Triangle &triangle)
{
    std::vector<std::pair<TriPoint, Point>> triangulation_input;
    std::vector<Triangle_intersection> intersections;

    tree.all_intersections(triangle, std::back_inserter(intersections));

    const Point verts[] = {triangle.vertex(0), triangle.vertex(1), triangle.vertex(2)};
    auto basis_v1 = verts[1] - verts[0];
    auto basis_v2 = verts[2] - verts[0];

    // Include original triangle points in triangulation
    for (int i = 0; i < 3; i++)
    {
        auto v = verts[i];
        auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
        auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
        triangulation_input.push_back(std::make_pair(TriPoint(x, y), v));
    }

    // Include intersection result
    for (auto const &ti : intersections)
    {
        if (auto intersection_point = boost::get<Point>(&(ti->first)))
        {
            auto v = *intersection_point;
            auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
            auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
            triangulation_input.push_back(std::make_pair(TriPoint(x, y), v));
        }
        else if (auto intersection_segment = boost::get<Segment>(&(ti->first)))
        {
            for (int i = 0; i < 2; i++)
            {
                auto v = intersection_segment->vertex(i);
                auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
                auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
                triangulation_input.push_back(std::make_pair(TriPoint(x, y), v));
            }
        }
        else if (auto intersection_triangle = boost::get<Triangle>(&(ti->first)))
        {
            for (int i = 0; i < 3; i++)
            {
                auto v = intersection_triangle->vertex(i);
                auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
                auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
                triangulation_input.push_back(std::make_pair(TriPoint(x, y), v));
            }
        }
        else if (auto intersection_polygon = boost::get<std::vector<Point>>(&(ti->first)))
        {
            for (auto const &v : *intersection_polygon)
            {
                auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
                auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
                triangulation_input.push_back(std::make_pair(TriPoint(x, y), v));
            }
        }
    }

    Triangulation triangulation(triangulation_input.begin(), triangulation_input.end());
    std::vector<Point> output;
    for (auto it = triangulation.finite_faces_begin(); it != triangulation.finite_faces_end(); it++)
    {
        for (int i = 0; i < 3; i++)
        {
            output.push_back(it->vertex(i)->info());
        }
    }
    return output;
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
        auto tessellation = triangulation_from_intersection(tree_1, tri);
        std::cout << "Tessellation: " << std::endl;
        for (auto const &p : tessellation)
        {
            std::cout << p << std::endl;
        }
    }

    return 0;
}
