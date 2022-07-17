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
#include <unordered_map>
#include <mutex>
#include <tbb/concurrent_vector.h>

#include "stl_io.hh"
#include "vec3.hh"
#include "indexed_mesh.hh"
#include "cgal_types.hh"

/* Point of intersection alongside indices of the mesh and triangle it came from */
struct IntersectionPoint
{
    unsigned geomID, primID;
    Point p;
};

struct CGALTri
{
    Triangle t;
    Segment segments[3];
};

struct IntersectionData
{
    std::mutex mutex;
    std::vector<stl::Triangle> tri_soup;
    tbb::concurrent_vector<IntersectionPoint> intersection_points;
    std::vector<CGALTri> cgal_tris;
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
        const auto &t2_cgal = data->cgal_tris[primID1].t;

        for (int si = 0; si < 3; si++)
        {
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
                data->intersection_points.push_back({geomID0, primID0, result_segment->vertex(0)});
                data->intersection_points.push_back({geomID0, primID0, result_segment->vertex(1)});
            }
            else if (auto result_point = boost::get<Point>(&(*result)))
            {
                data->intersection_points.push_back({geomID0, primID0, *result_point});
            }
        }
    }
}

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
