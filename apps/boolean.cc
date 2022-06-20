#include <chrono>
#include <iostream>
#include <vector>
#include <cmath>
#include <array>
#include <fstream>

#include "stl_io.hh"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include <CGAL/Triangulation_2.h>

// typedef CGAL::Simple_cartesian<double> K;

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Triangulation_vertex_base_with_info_2<int, K> Vb;

// typedef CGAL::Projection_traits_xy_3<K> Gt;
// typedef CGAL::Delaunay_triangulation_2<Gt> Delaunay;

typedef CGAL::Triangulation_data_structure_2<Vb> Tds;
typedef CGAL::Delaunay_triangulation_2<K, Tds> Delaunay;

typedef K::FT FT;
typedef K::Ray_3 Ray;
typedef K::Line_3 Line;
typedef K::Point_3 Point3;
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
        Point3 p1(t.v1[0], t.v1[1], t.v1[2]);
        Point3 p2(t.v2[0], t.v2[1], t.v2[2]);
        Point3 p3(t.v3[0], t.v3[1], t.v3[2]);

        mesh_1.push_back(Triangle(p1, p2, p3));
    }
    std::list<Triangle> mesh_2;
    for (auto const &t : load_stl(argv[2]))
    {
        Point3 p1(t.v1[0], t.v1[1], t.v1[2]);
        Point3 p2(t.v2[0], t.v2[1], t.v2[2]);
        Point3 p3(t.v3[0], t.v3[1], t.v3[2]);

        mesh_2.push_back(Triangle(p1, p2, p3));
    }

    Tree tree_1(mesh_1.begin(), mesh_1.end());

    std::vector<mp::io::stl::Triangle> output_tris;

    for (auto const &t2 : mesh_2)
    {
        auto v1 = t2.vertex(0);
        auto v2 = t2.vertex(1);
        auto v3 = t2.vertex(2);

        auto e1 = v2 - v1;
        auto e2 = v3 - v1;

        std::vector<Triangle_intersection> intersections;
        tree_1.all_intersections(t2, std::back_inserter(intersections));

        std::vector<std::pair<Point2, int>> delaunay_input_points;
        for (int i = 0; i < intersections.size(); i++)
        {
            auto const &ti = intersections[i];
            auto p = boost::get<Point3>(&(ti->first));
            if (p)
            {
                auto x_2d = p->x() * e1.x() + p->y() * e1.y() + p->z() * e1.z();
                auto y_2d = p->x() * e2.x() + p->y() * e2.y() + p->z() * e2.z();

                delaunay_input_points.push_back(std::make_pair(Point2(x_2d, y_2d), i));
            }
        }

        Delaunay dt(delaunay_input_points.begin(), delaunay_input_points.end());

        mp::io::stl::Triangle tri_buf{};
        for (auto it = dt.finite_faces_begin(); it != dt.finite_faces_end(); it++)
        {

            auto v1_i = it->vertex(0)->info();
            auto v2_i = it->vertex(1)->info();
            auto v3_i = it->vertex(2)->info();

            auto p1 = boost::get<Point3>(&(intersections[v1_i]->first));
            auto p2 = boost::get<Point3>(&(intersections[v2_i]->first));
            auto p3 = boost::get<Point3>(&(intersections[v3_i]->first));

            // std::cout << "Triangle: " << (*p1) << ", " << (*p2) << ", " << (*p3) << std::endl;

            if (p1 && p2 && p3)
            {
                for (int i = 0; i < 3; i++)
                {
                    tri_buf.v1[i] = p1->cartesian(i);
                }
                for (int i = 0; i < 3; i++)
                {
                    tri_buf.v2[i] = p2->cartesian(i);
                }
                for (int i = 0; i < 3; i++)
                {
                    tri_buf.v3[i] = p3->cartesian(i);
                }
                output_tris.push_back(tri_buf);
            }
        }
    }

    std::cout << "Num out tris = " << output_tris.size() << std::endl;

    write_stl(output_tris, "boolean_output.stl");

    auto tris = load_stl(argv[1]);
    mp::io::stl::write_stl(tris, "foo.stl");

    return 0;
}
