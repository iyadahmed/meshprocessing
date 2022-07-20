#pragma once

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define likely(expr) (expr)
#define unlikely(expr) (expr)
#else
#define likely(expr) __builtin_expect((bool)(expr), true)
#define unlikely(expr) __builtin_expect((bool)(expr), false)
#endif

#ifdef NDEBUG
#define tassert(x)
#else
#define tassert(x) \
    if (!(x))      \
    {              \
        throw;     \
    }
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
std::pair<Triangulation::Point, TriangulationPointInfo> to_ip(const Point &p, const Triangle &t)
{
    TriangulationPointInfo info{p};
    auto p2d = project_point(p, t);
    return {p2d, info};
}

// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
inline void barycentric(const Vec3 &p, const Vec3 &a, const Vec3 &b, const Vec3 &c, float &u, float &v, float &w)
{
    Vec3 v0 = b - a; // TODO: compute once per triangle
    Vec3 v1 = c - a; // TODO: compute once per triangle
    Vec3 v2 = p - a;
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}

inline Vec3 to_barycentric(const Vec3 &p, const Vec3 &a, const Vec3 &b, const Vec3 &c)
{
    float u, v, w;
    barycentric(p, a, b, c, u, v, w);
    return {u, v, w};
}

inline bool is_inside(const Vec3 &p, const Vec3 &a, const Vec3 &b, const Vec3 &c, float tolerance)
{
    float u, v, w;
    barycentric(p, a, b, c, u, v, w);
    float upper = 1.0f + tolerance;
    float lower = 0.0f - tolerance;
    return (u >= lower) && (u <= upper) && (v >= lower) && (v <= upper) && (w >= lower) && (w <= upper);
}

int side(Vec3 p, Vec3 a, Vec3 b, Vec3 c, float tolerance)
{
    Vec3 v0 = b - a;            // TODO: compute once per triangle
    Vec3 v1 = c - a;            // TODO: compute once per triangle
    Vec3 normal = v0.cross(v1); // TODO: compute once per triangle
    Vec3 v2 = p - a;
    float d = v2.dot(normal);
    if (std::abs(d) <= tolerance)
    {
        return true;
    }

    return (d > 0.0f) ? 1 : -1;
}

Vec3 to_vec3(Point p)
{
    return {(float)CGAL::to_double(p.x()),
            (float)CGAL::to_double(p.y()),
            (float)CGAL::to_double(p.z())};
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
// TODO: get rid of CGAL types
// TODO: use double precision
#define TOL .0001f
            const auto &s = t1.segments[si];
            auto side1 = side(to_vec3(s.vertex(0)),
                              to_vec3(t2_cgal.vertex(0)),
                              to_vec3(t2_cgal.vertex(1)),
                              to_vec3(t2_cgal.vertex(2)),
                              TOL);
            auto side2 = side(to_vec3(s.vertex(1)),
                              to_vec3(t2_cgal.vertex(0)),
                              to_vec3(t2_cgal.vertex(1)),
                              to_vec3(t2_cgal.vertex(2)),
                              TOL);
            if ((side1 == 0) && (side2 == 0))
            {
                // Edge is coplanar with triangle2
                auto sv1_is_inside = is_inside(to_vec3(s.vertex(0)),
                                               to_vec3(t2_cgal.vertex(0)),
                                               to_vec3(t2_cgal.vertex(1)),
                                               to_vec3(t2_cgal.vertex(2)),
                                               TOL);
                auto sv2_is_inside = is_inside(to_vec3(s.vertex(1)),
                                               to_vec3(t2_cgal.vertex(0)),
                                               to_vec3(t2_cgal.vertex(1)),
                                               to_vec3(t2_cgal.vertex(2)),
                                               TOL);
                if (sv1_is_inside && sv2_is_inside)
                {
                    data->intersection_points_map[primID0].push_back(to_ip(s.vertex(0), t1_cgal));
                    data->intersection_points_map[primID0].push_back(to_ip(s.vertex(1), t1_cgal));

                    data->intersection_points_map[primID1].push_back(to_ip(s.vertex(0), t2_cgal));
                    data->intersection_points_map[primID1].push_back(to_ip(s.vertex(1), t2_cgal));
                }
                else
                {
                    auto t2_plane = t2_cgal.supporting_plane();
                    auto sv1_p = t2_plane.projection(s.vertex(0));
                    auto sv2_p = t2_plane.projection(s.vertex(1));
                    Segment projected_segment(sv1_p, sv2_p);
                    if (!CGAL::do_intersect(projected_segment, t2_cgal))
                    {
                        continue;
                    }
                    auto result = CGAL::intersection(projected_segment, t2_cgal);
                    if (!result)
                    {
                        continue;
                    }
                    if (auto result_segment = boost::get<Segment>(&(*result)))
                    {
                        data->intersection_points_map[primID0].push_back(to_ip(result_segment->vertex(0), t1_cgal));
                        data->intersection_points_map[primID0].push_back(to_ip(result_segment->vertex(1), t1_cgal));

                        data->intersection_points_map[primID1].push_back(to_ip(result_segment->vertex(0), t2_cgal));
                        data->intersection_points_map[primID1].push_back(to_ip(result_segment->vertex(1), t2_cgal));
                    }
                    else if (auto result_point = boost::get<Point>(&(*result)))
                    {
                        data->intersection_points_map[primID0].push_back(to_ip(*result_point, t1_cgal));
                        data->intersection_points_map[primID1].push_back(to_ip(*result_point, t2_cgal));
                    }
                }
            }
            else if (side1 != side2)
            {
                auto t2_plane = t2_cgal.supporting_plane();
                K::Ray_3 ray(s.vertex(0), s.vertex(1));
                tassert(CGAL::do_intersect(ray, t2_plane));
                auto result = CGAL::intersection(ray, t2_plane);
                tassert(result);
                if (auto result_point = boost::get<Point>(&(*result)))
                {
                    auto d = is_inside(to_vec3(*result_point),
                                       to_vec3(t2_cgal.vertex(0)),
                                       to_vec3(t2_cgal.vertex(1)),
                                       to_vec3(t2_cgal.vertex(2)),
                                       TOL);
                    if (d)
                    {
                        data->intersection_points_map[primID0].push_back(to_ip(*result_point, t1_cgal));
                        data->intersection_points_map[primID1].push_back(to_ip(*result_point, t2_cgal));
                    }
                }
                else
                {
                    throw;
                }
            }
            else
            {
                // Else case explicitly not handled
                // as intersection is impossible if both segment vertices are on the same side of triangle plane
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
