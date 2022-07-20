#pragma once

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define likely(expr) (expr)
#define unlikely(expr) (expr)
#else
#define likely(expr) __builtin_expect((bool)(expr), true)
#define unlikely(expr) __builtin_expect((bool)(expr), false)
#endif

#include <embree3/rtcore.h>
#include <vector>
#include <mutex>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_map.h>

#include "stl_io.hh"
#include "vec3.hh"
#include "indexed_mesh.hh"
#include "cgal_types.hh"

struct CGALTri
{
    Triangle t;
    Segment segments[3];
};


// Stores input and output data for rtcCollide,
// must be allocated on memory that can be shared between threads,
// like the heap
struct IntersectionData
{
    std::mutex mutex;
    std::vector<stl::Triangle> tri_soup;
    std::vector<CGALTri> cgal_tris;
    // NOTE: you can get rid of the map of vectors, by storing in a linear vector of intersection points,
    // then after collision is finished, you can sort that vector by primitive id,
    // so that all points that belong to the same triangle are consequtive,
    // make sure to store the primtive id along side each point so you can use it to sort,
    // also make sure to store two copies of the intersection point, 2 intersection points for each two intersecting triangles
    // a map of vectors was chosen in the end because it reduced code complexity, and had near zero impact on performance
    tbb::concurrent_unordered_map<unsigned int, tbb::concurrent_vector<std::pair<Triangulation::Point, TriangulationPointInfo>>> intersection_points_map;
};

inline Triangulation::Point project_point(const Point &a, const Triangle &t)
{
    auto basis_v1 = t.vertex(1) - t.vertex(0);
    auto basis_v2 = t.vertex(2) - t.vertex(0);
    auto x = CGAL::scalar_product(a - CGAL::ORIGIN, basis_v1);
    auto y = CGAL::scalar_product(a - CGAL::ORIGIN, basis_v2);
    return {x, y};
}

inline void collide_func_cgal_tris(void *user_data_ptr, RTCCollision *collisions, unsigned int num_collisions)
{
    if (num_collisions == 0)
        return;

    IntersectionData *data = (IntersectionData *)user_data_ptr;
    for (size_t i = 0; i < num_collisions; i++)
    {
        const unsigned &primID0 = collisions[i].primID0;
        const unsigned &primID1 = collisions[i].primID1;
        const unsigned &geomID0 = collisions[i].geomID0;
        const unsigned &geomID1 = collisions[i].geomID1;

        if (primID0 == primID1)
        {
            continue;
        }

        const auto &t1 = data->cgal_tris[primID0];
        const auto &t1_cgal = t1.t;
        const auto &t2_cgal = data->cgal_tris[primID1].t;

        for (int si = 0; si < 3; si++)
        {
            // TODO: write a better intersection "kernel"
            // that handles coplanar cases and has tolerance
            const auto &s = t1.segments[si];
            if (is_linked_to_segment(t2_cgal, s))
            {
                continue;
            }
            if (!CGAL::do_intersect(s, t2_cgal))
            {
                continue;
            }
            auto result = CGAL::intersection(s, t2_cgal);
            if (!result)
            {
                continue;
            }
            if (auto result_segment = boost::get<Segment>(&(*result)))
            {
                auto a_3d = result_segment->vertex(0);
                TriangulationPointInfo a_info{a_3d};
                auto a_2d_t1 = project_point(a_3d, t1_cgal);
                auto a_2d_t2 = project_point(a_3d, t2_cgal);

                auto b_3d = result_segment->vertex(1);
                TriangulationPointInfo b_info{b_3d};
                auto b_2d_t1 = project_point(b_3d, t1_cgal);
                auto b_2d_t2 = project_point(b_3d, t2_cgal);
                data->intersection_points_map[primID0].push_back({a_2d_t1, a_info});
                data->intersection_points_map[primID0].push_back({b_2d_t1, b_info});

                data->intersection_points_map[primID1].push_back({a_2d_t2, a_info});
                data->intersection_points_map[primID1].push_back({b_2d_t2, b_info});
            }
            else if (auto result_point = boost::get<Point>(&(*result)))
            {
                auto a_3d = *result_point;
                TriangulationPointInfo a_info{a_3d};
                auto a_2d_t1 = project_point(a_3d, t1_cgal);
                auto a_2d_t2 = project_point(a_3d, t2_cgal);

                data->intersection_points_map[primID0].push_back({a_2d_t1, a_info});
                data->intersection_points_map[primID1].push_back({a_2d_t2, a_info});
            }
        }
    }
}

// TODO: should a tolerance be added here?
void triangle_bounds_func_cgal_tris(const struct RTCBoundsFunctionArguments *args)
{
    IntersectionData *data = (IntersectionData *)args->geometryUserPtr;
    const auto &t = data->cgal_tris[args->primID].t;
    args->bounds_o->lower_x = t.bbox().min(0);
    args->bounds_o->lower_y = t.bbox().min(1);
    args->bounds_o->lower_z = t.bbox().min(2);

    args->bounds_o->upper_x = t.bbox().max(0);
    args->bounds_o->upper_y = t.bbox().max(1);
    args->bounds_o->upper_z = t.bbox().max(2);
}
