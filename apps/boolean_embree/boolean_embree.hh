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

using namespace mp::io;

/* Point of intersection alongside indices of the mesh and triangle it came from */
struct IntersectionPoint
{
    unsigned geomID, primID;
    Point p;
};

struct IntersectionData
{
    std::mutex mutex;
    std::vector<stl::Triangle> tri_soup;
    // std::vector<IntersectionPoint> intersection_points;
    tbb::concurrent_vector<IntersectionPoint> intersection_points;
    std::unordered_map<unsigned int, std::vector<IntersectionPoint>> intersection_points_map;
    // IndexedMesh mesh;
    std::vector<Triangle> cgal_tris;
};

inline Triangulation::Point project_point(const Point &a, const Triangle &t)
{
    auto basis_v1 = t.vertex(1) - t.vertex(0);
    auto basis_v2 = t.vertex(2) - t.vertex(0);
    auto x = CGAL::scalar_product(a - CGAL::ORIGIN, basis_v1);
    auto y = CGAL::scalar_product(a - CGAL::ORIGIN, basis_v2);
    return {x, y};
}

inline Triangle to_cgal_triangle(const stl::Triangle &t)
{
    return {
        {t.v1[0], t.v1[1], t.v1[2]},
        {t.v2[0], t.v2[1], t.v2[2]},
        {t.v3[0], t.v3[1], t.v3[2]},
    };
}

inline Triangle to_cgal_triangle(const Vec3 &a, const Vec3 &b, const Vec3 &c)
{
    return {
        {a.x, a.y, a.z},
        {b.x, b.y, b.z},
        {c.x, c.y, c.z},
    };
}

inline Segment to_cgal_segment(const Vec3 &a, const Vec3 &b)
{
    return {
        {a.x, a.y, a.z},
        {b.x, b.y, b.z},
    };
}

// inline bool is_tet_positive(const Vec3 &a, const Vec3 &b, const Vec3 &c, const Vec3 &d)
// {
//     return (a - d).dot((b - d).cross(c - d)) > 0;
// }

// inline bool do_intersect_segment_tri(const Vec3 &segment_a,
//                                      const Vec3 &segment_b,
//                                      const Vec3 &tri_a,
//                                      const Vec3 &tri_b,
//                                      const Vec3 &tri_c)
// {
//     bool a = is_tet_positive(segment_a, tri_a, tri_b, tri_c);
//     bool b = is_tet_positive(segment_b, tri_a, tri_b, tri_c);

//     bool c = is_tet_positive(segment_a, segment_b, tri_a, tri_b);
//     bool d = is_tet_positive(segment_b, segment_b, tri_b, tri_c);
//     bool e = is_tet_positive(segment_b, segment_b, tri_c, tri_a);

//     return (a != b) && (c == d) && (d == e);
// }

inline void collide_func_process_intersection(
    IntersectionData *data,
    const boost::optional<boost::variant<Point, Segment>> &result,
    unsigned int primID0,
    unsigned int primID1,
    unsigned int geomID0,
    unsigned int geomID1)
{
    auto &out = data->intersection_points_map;
    if (result)
    {
        if (auto p = boost::get<Point>(&(*result)))
        {
            data->intersection_points.push_back({geomID0, primID0, *p});
            data->intersection_points.push_back({geomID1, primID1, *p});
        }
        else if (auto s = boost::get<Segment>(&(*result)))
        {
            for (int i = 0; i < 2; i++)
            {
                data->intersection_points.push_back({geomID0, primID0, s->vertex(i)});
                data->intersection_points.push_back({geomID1, primID1, s->vertex(i)});
            }
        }
    }
}

// inline void collide_func_indexed_mesh(void *user_data_ptr, RTCCollision *collisions, unsigned int num_collisions)
// {
//     if (num_collisions == 0)
//         return;

//     IntersectionData *data = (IntersectionData *)user_data_ptr;
//     for (size_t i = 0; i < num_collisions; i++)
//     {
//         const unsigned &primID0 = collisions[i].primID0;
//         const unsigned &primID1 = collisions[i].primID1;
//         const unsigned &geomID0 = collisions[i].geomID0;
//         const unsigned &geomID1 = collisions[i].geomID1;

//         if (primID0 == primID1)
//         {
//             continue;
//         }
//         const auto &t1 = data->mesh.tris[primID0];
//         const auto &t2 = data->mesh.tris[primID1];

//         const auto &verts = data->mesh.verts;

//         const auto &t2_v1 = verts[t2.v1];
//         const auto &t2_v2 = verts[t2.v2];
//         const auto &t2_v3 = verts[t2.v3];

//         const auto &t2_cgal = to_cgal_triangle(t2_v1, t2_v2, t2_v3);

//         std::pair<int, int> segments[3] = {{t1.v1, t1.v2}, {t1.v2, t1.v3}, {t1.v3, t1.v1}};
//         for (int j = 0; j < 3; j++)
//         {
//             auto [v1, v2] = segments[j];
//             if (t2.has_edge(v1, v2))
//             {
//                 continue;
//             }
//             auto s_cgal = to_cgal_segment(verts[v1], verts[v2]);
//             if (CGAL::do_intersect(s_cgal, t2_cgal))
//             {
//                 const auto &result = CGAL::intersection(s_cgal, t2_cgal);
//                 collide_func_process_intersection(data, result, primID0, primID1, geomID0, geomID1);
//             }
//         }
//     }
// }

// inline void collide_func(void *user_data_ptr, RTCCollision *collisions, unsigned int num_collisions)
// {
//     if (num_collisions == 0)
//         return;

//     IntersectionData *data = (IntersectionData *)user_data_ptr;

//     for (size_t i = 0; i < num_collisions; i++)
//     {
//         const unsigned &primID0 = collisions[i].primID0;
//         const unsigned &primID1 = collisions[i].primID1;

//         const unsigned &geomID0 = collisions[i].geomID0;
//         const unsigned &geomID1 = collisions[i].geomID1;

//         const stl::Triangle &t1 = data->tri_soup[primID0];
//         const stl::Triangle &t2 = data->tri_soup[primID1];

//         Vec3 *verts1 = (Vec3 *)t1.verts;
//         Vec3 *verts2 = (Vec3 *)t2.verts;

//         float d1 = (verts1[0] - verts2[0]).length_squared();
//         float d2 = (verts1[1] - verts2[0]).length_squared();
//         float d3 = (verts1[2] - verts2[0]).length_squared();

//         for (int i = 1; i < 2; i++)
//         {
//             d1 = std::min(d1, (verts1[0] - verts2[i]).length_squared());
//             d2 = std::min(d1, (verts1[1] - verts2[i]).length_squared());
//             d3 = std::min(d1, (verts1[2] - verts2[i]).length_squared());
//         }

//         bool v1_supports_t2 = std::abs(d1) < .00001;
//         bool v2_supports_t2 = std::abs(d2) < .00001;
//         bool v3_supports_t2 = std::abs(d3) < .00001;

//         bool s_supports_t2[3];
//         s_supports_t2[0] = (v1_supports_t2 && v2_supports_t2);
//         s_supports_t2[1] = (v2_supports_t2 && v3_supports_t2);
//         s_supports_t2[2] = (v3_supports_t2 && v1_supports_t2);

//         const auto &t2_cgal = to_cgal_triangle(t2);
//         Segment segments[3];
//         segments[0] = to_cgal_segment(verts1[0], verts1[1]);
//         segments[1] = to_cgal_segment(verts1[1], verts1[2]);
//         segments[2] = to_cgal_segment(verts1[2], verts1[0]);
//         for (int j = 0; j < 3; j++)
//         {
//             if (s_supports_t2[j])
//             {
//                 // Skip if segment belongs to triangle
//                 continue;
//             }
//             if (!CGAL::do_intersect(segments[j], t2_cgal))
//             {
//                 continue;
//             }
//             const auto &result = CGAL::intersection(segments[j], t2_cgal);
//             {
//                 std::scoped_lock lock(data->mutex);
//                 if (result)
//                 {
//                     if (auto intersection_point = boost::get<Point>(&(*result)))
//                     {
//                         // data->intersection_points.push_back({geomID0, primID0, *intersection_point});
//                         // data->intersection_points.push_back({geomID1, primID1, *intersection_point});
//                         data->intersection_points_map[primID0].push_back({geomID0, primID0, *intersection_point});
//                         data->intersection_points_map[primID1].push_back({geomID1, primID1, *intersection_point});
//                     }
//                     if (auto intersection_segment = boost::get<Segment>(&(*result)))
//                     {
//                         // data->intersection_points.push_back({geomID0, primID0, intersection_segment->vertex(0)});
//                         // data->intersection_points.push_back({geomID0, primID0, intersection_segment->vertex(1)});

//                         // data->intersection_points.push_back({geomID1, primID1, intersection_segment->vertex(0)});
//                         // data->intersection_points.push_back({geomID1, primID1, intersection_segment->vertex(1)});

//                         data->intersection_points_map[primID0].push_back({geomID0, primID0, intersection_segment->vertex(0)});
//                         data->intersection_points_map[primID0].push_back({geomID0, primID0, intersection_segment->vertex(1)});

//                         data->intersection_points_map[primID1].push_back({geomID1, primID1, intersection_segment->vertex(0)});
//                         data->intersection_points_map[primID1].push_back({geomID1, primID1, intersection_segment->vertex(1)});
//                     }
//                 }
//             }
//         }
//     }
// }

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
        const auto &t2 = data->cgal_tris[primID1];
        if (CGAL::do_intersect(t1, t2))
        {
            auto result = CGAL::intersection(t1, t2);
            if (result)
            {
                if (auto s = boost::get<Segment>(&(*result)))
                {
                    data->intersection_points.push_back({geomID0, primID0, s->vertex(0)});
                    data->intersection_points.push_back({geomID0, primID0, s->vertex(1)});
                }
            }
        }
    }
}

void triangle_bounds_func_cgal_tris(const struct RTCBoundsFunctionArguments *args)
{
    IntersectionData *data = (IntersectionData *)args->geometryUserPtr;
    const auto &t = data->cgal_tris[args->primID];
    args->bounds_o->lower_x = t.bbox().min(0);
    args->bounds_o->lower_y = t.bbox().min(1);
    args->bounds_o->lower_z = t.bbox().min(2);

    args->bounds_o->upper_x = t.bbox().max(0);
    args->bounds_o->upper_y = t.bbox().max(1);
    args->bounds_o->upper_z = t.bbox().max(2);
}
