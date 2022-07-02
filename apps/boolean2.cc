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

struct BooleanTriangle
{
    Point a;
    Point b;
    Point c;
    int mesh_index;
};

typedef std::vector<BooleanTriangle>::const_iterator Iterator;

struct BooleanTrianglePrimitive
{
public:
    typedef const BooleanTriangle *Id;
    typedef TreeK::Point_3 Point;
    typedef TreeK::Triangle_3 Datum;

private:
    Id m_pt;

public:
    BooleanTrianglePrimitive() {}
    BooleanTrianglePrimitive(Iterator it)
        : m_pt(&(*it)) {}
    const Id &id() const { return m_pt; }

    Datum datum() const
    {
        return Datum(m_pt->a,
                     m_pt->b,
                     m_pt->c);
    }
    Point reference_point() const
    {
        return m_pt->a;
    }
};

struct TriangulationPointInfo
{
    Point point_3d;
    int mesh_index;
};

// typedef std::vector<Triangle>::iterator Iterator;

// typedef CGAL::AABB_triangle_primitive<TreeK, Iterator> Primitive;

typedef CGAL::AABB_traits<TreeK, BooleanTrianglePrimitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;
typedef Tree::Primitive_id Primitive_id;

typedef boost::optional<Tree::Intersection_and_primitive_id<Triangle>::Type> Triangle_intersection;

typedef CGAL::Exact_predicates_inexact_constructions_kernel TriK;
typedef CGAL::Triangulation_vertex_base_with_info_2<TriangulationPointInfo, TriK> Vb;
typedef CGAL::Triangulation_data_structure_2<Vb> Tds;

typedef CGAL::Triangulation_2<TriK, Tds> Triangulation;
typedef Triangulation::Point TriPoint;

static std::vector<mp::io::stl::Triangle> load_stl(const char *filepath)
{
    std::vector<mp::io::stl::Triangle> tris;
    mp::io::stl::read_stl(filepath, tris);
    return tris;
}

// Computes new triangles from intersection points of a triangle with a Tree
static Triangulation triangulation_from_intersection(const Tree &tree, const BooleanTriangle &bool_triangle)
{
    std::vector<std::pair<TriPoint, TriangulationPointInfo>> triangulation_input;
    std::vector<Triangle_intersection> intersections;

    auto triangle = Triangle(bool_triangle.a, bool_triangle.b, bool_triangle.c);

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
        triangulation_input.push_back(std::make_pair(TriPoint(x, y), TriangulationPointInfo{v, bool_triangle.mesh_index}));
    }

    // Include intersection result
    for (auto const &ti : intersections)
    {
        if (auto intersection_point = boost::get<Point>(&(ti->first)))
        {
            auto v = *intersection_point;
            auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
            auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
            triangulation_input.push_back(std::make_pair(TriPoint(x, y), TriangulationPointInfo{v, bool_triangle.mesh_index}));
        }
        else if (auto intersection_segment = boost::get<Segment>(&(ti->first)))
        {
            for (int i = 0; i < 2; i++)
            {
                auto v = intersection_segment->vertex(i);
                auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
                auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
                triangulation_input.push_back(std::make_pair(TriPoint(x, y), TriangulationPointInfo{v, bool_triangle.mesh_index}));
            }
        }
        else if (auto intersection_triangle = boost::get<Triangle>(&(ti->first)))
        {
            for (int i = 0; i < 3; i++)
            {
                auto v = intersection_triangle->vertex(i);
                auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
                auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
                triangulation_input.push_back(std::make_pair(TriPoint(x, y), TriangulationPointInfo{v, bool_triangle.mesh_index}));
            }
        }
        else if (auto intersection_polygon = boost::get<std::vector<Point>>(&(ti->first)))
        {
            for (auto const &v : *intersection_polygon)
            {
                auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
                auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
                triangulation_input.push_back(std::make_pair(TriPoint(x, y), TriangulationPointInfo{v, bool_triangle.mesh_index}));
            }
        }
    }

    return Triangulation(triangulation_input.begin(), triangulation_input.end());
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        puts("Usage: boolean a.stl b.stl");
        return 1;
    }

    std::cout << "CGAL Version: " << CGAL_VERSION_STR << std::endl;

    // TODO: refactor stl_read to accept call back to handle read triangles from file as they are read

    std::vector<BooleanTriangle> boolean_tri_soup;
    for (auto const &t : load_stl(argv[1]))
    {
        Point p1(t.v1[0], t.v1[1], t.v1[2]);
        Point p2(t.v2[0], t.v2[1], t.v2[2]);
        Point p3(t.v3[0], t.v3[1], t.v3[2]);

        boolean_tri_soup.push_back({p1, p2, p3, 0});
    }
    for (auto const &t : load_stl(argv[2]))
    {
        Point p1(t.v1[0], t.v1[1], t.v1[2]);
        Point p2(t.v2[0], t.v2[1], t.v2[2]);
        Point p3(t.v3[0], t.v3[1], t.v3[2]);

        boolean_tri_soup.push_back({p1, p2, p3, 1});
    }
    Tree tree(boolean_tri_soup.begin(), boolean_tri_soup.end());

    std::vector<mp::io::stl::Triangle> out;
    mp::io::stl::Triangle tri_buf;

    for (auto const &bt : boolean_tri_soup)
    {
        auto triangulation = triangulation_from_intersection(tree, bt);
        for (auto it = triangulation.finite_faces_begin(); it != triangulation.finite_faces_end(); it++)
        {
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    tri_buf.verts[i][j] = it->vertex(i)->info().point_3d[j];
                }
            }
            // TODO: Filter triangles based on winding number of one their points
            // and the mesh they came from
            out.push_back(tri_buf);
        }
    }

    // TODO: refactor stl write to write triangles on demand
    mp::io::stl::write_stl(out, "out.stl");

    return 0;
}
