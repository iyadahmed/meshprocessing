#pragma once

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef K::Triangle_3 Triangle;
typedef K::Point_3 Point;
typedef K::Segment_3 Segment;

void cgal_tri_tri_intersection_points(std::vector<Point> &out, const Triangle &t1, const Triangle &t2)
{
    auto intersection_opt = CGAL::intersection(t1, t2);
    if (!intersection_opt)
    {
        return;
    }
    auto intersection = &(*intersection_opt);
    if (auto intersection_point = boost::get<Point>(intersection))
    {
        out.push_back(*intersection_point);
        // auto v = *intersection_point;
        // auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
        // auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
        // triangulation_input.push_back(std::make_pair(TriPoint(x, y), TriangulationPointInfo{v, bool_triangle.mesh_index}));
    }
    else if (auto intersection_segment = boost::get<Segment>(intersection))
    {
        for (int i = 0; i < 2; i++)
        {
            out.push_back(intersection_segment->vertex(i));
            // auto v = intersection_segment->vertex(i);
            // auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
            // auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
            // triangulation_input.push_back(std::make_pair(TriPoint(x, y), TriangulationPointInfo{v, bool_triangle.mesh_index}));
        }
    }
    else if (auto intersection_triangle = boost::get<Triangle>(intersection))
    {
        for (int i = 0; i < 3; i++)
        {
            out.push_back(intersection_triangle->vertex(i));
            // auto v = intersection_triangle->vertex(i);
            // auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
            // auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
            // triangulation_input.push_back(std::make_pair(TriPoint(x, y), TriangulationPointInfo{v, bool_triangle.mesh_index}));
        }
    }
    else if (auto intersection_polygon = boost::get<std::vector<Point>>(intersection))
    {
        for (auto const &v : *intersection_polygon)
        {
            out.push_back(v);
            // auto x = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v1);
            // auto y = CGAL::scalar_product(v - CGAL::ORIGIN, basis_v2);
            // triangulation_input.push_back(std::make_pair(TriPoint(x, y), TriangulationPointInfo{v, bool_triangle.mesh_index}));
        }
    }
}
