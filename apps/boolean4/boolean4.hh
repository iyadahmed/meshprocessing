#pragma once

// Fixes CGAL assert when running debug build under valgrind
#define CGAL_DISABLE_ROUNDING_MATH_CHECK

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

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>

#include "stl_io.hh"
#include "vec3.hh"

// Use exact predicates and constructions to avoid precondition exception (degenerate edges being generated while intersecting triangles)
// Also for better precision and handling coplanar cases
typedef CGAL::Exact_predicates_exact_constructions_kernel K;
// typedef CGAL::Simple_cartesian<double> K;
typedef K::Triangle_3 Triangle;
typedef K::Segment_3 Segment;

using namespace mp::io;

struct BBox3fa
{
    Vec3 upper = -std::numeric_limits<float>::infinity();
    Vec3 lower = std::numeric_limits<float>::infinity();
    void extend(const Vec3 &point)
    {
        lower.min(point);
        upper.max(point);
    }
};

struct GeometryPoint
{
    unsigned geomID, primID;
};

struct IntersectionPair
{
    GeometryPoint a, b;
};

/* Self-intersection data */
struct Data
{
    std::mutex mutex;
    std::vector<stl::Triangle> tri_soup;
    std::vector<IntersectionPair> intersections;
};

inline Triangle to_cgal_triangle(const stl::Triangle &t)
{
    return {
        {t.v1[0], t.v1[1], t.v1[2]},
        {t.v2[0], t.v2[1], t.v2[2]},
        {t.v3[0], t.v3[1], t.v3[2]},
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

inline bool do_intersect(const std::vector<stl::Triangle> &tri_soup, unsigned geomID0, unsigned primID0, unsigned geomID1, unsigned primID1)
{
    if (primID0 == primID1)
    {
        return false;
    }

    const stl::Triangle &t1 = tri_soup[primID0];
    const stl::Triangle &t2 = tri_soup[primID1];

    Vec3 *verts1 = (Vec3 *)t1.verts;
    Vec3 *verts2 = (Vec3 *)t2.verts;

    float d1 = (verts1[0] - verts2[0]).length_squared();
    for (int i = 1; i < 2; i++)
    {
        d1 = std::min(d1, (verts1[0] - verts2[i]).length_squared());
    }

    float d2 = (verts1[1] - verts2[0]).length_squared();
    for (int i = 1; i < 2; i++)
    {
        d2 = std::min(d1, (verts1[1] - verts2[i]).length_squared());
    }

    float d3 = (verts1[2] - verts2[0]).length_squared();
    for (int i = 1; i < 2; i++)
    {
        d3 = std::min(d1, (verts1[2] - verts2[i]).length_squared());
    }

    bool v1_on_t2 = std::abs(d1) < .00001;
    bool v2_on_t2 = std::abs(d2) < .00001;
    bool v3_on_t2 = std::abs(d3) < .00001;

    bool s1_on_t2 = (v1_on_t2 && v2_on_t2);
    bool s2_on_t2 = (v2_on_t2 && v3_on_t2);
    bool s3_on_t2 = (v3_on_t2 && v1_on_t2);

    const Triangle &t2_cgal = to_cgal_triangle(t2);
    const Segment &s1 = to_cgal_segment(verts1[0], verts1[1]);
    const Segment &s2 = to_cgal_segment(verts1[1], verts1[2]);
    const Segment &s3 = to_cgal_segment(verts1[2], verts1[0]);

    if (!s1_on_t2)
    {
        if (CGAL::do_intersect(s1, t2_cgal))
        {
            return true;
        }
    }

    if (!s2_on_t2)
    {
        if (CGAL::do_intersect(s2, t2_cgal))
        {
            return true;
        }
    }

    if (!s3_on_t2)
    {
        if (CGAL::do_intersect(s3, t2_cgal))
        {
            return true;
        }
    }

    return false;
}

inline void collide_func(void *user_data_ptr, RTCCollision *collisions, unsigned int num_collisions)
{
    if (num_collisions == 0)
        return;

    Data *data = (Data *)user_data_ptr;
    for (size_t i = 0; i < num_collisions; i++)
    {
        bool is_intersection = do_intersect(data->tri_soup,
                                            collisions[i].geomID0, collisions[i].primID0,
                                            collisions[i].geomID1, collisions[i].primID1);
        if (is_intersection)
        {
            std::scoped_lock lock(data->mutex);
            data->intersections.push_back({{collisions[i].geomID0, collisions[i].primID0}, {collisions[i].geomID1, collisions[i].primID1}});
        }
    }
}

void triangle_bounds_func(const struct RTCBoundsFunctionArguments *args)
{
    Data *data = (Data *)args->geometryUserPtr;
    const stl::Triangle &t = data->tri_soup[args->primID];

    float &lower_x = args->bounds_o->lower_x;
    float &lower_y = args->bounds_o->lower_y;
    float &lower_z = args->bounds_o->lower_z;

    float &upper_x = args->bounds_o->upper_x;
    float &upper_y = args->bounds_o->upper_y;
    float &upper_z = args->bounds_o->upper_z;

    lower_x = INFINITY;
    lower_y = INFINITY;
    lower_z = INFINITY;

    upper_x = -INFINITY;
    upper_y = -INFINITY;
    upper_z = -INFINITY;

    for (int i = 0; i < 3; i++)
    {
        lower_x = std::min(lower_x, t.verts[i][0]);
        lower_y = std::min(lower_y, t.verts[i][1]);
        lower_z = std::min(lower_z, t.verts[i][2]);

        upper_x = std::max(upper_x, t.verts[i][0]);
        upper_y = std::max(upper_y, t.verts[i][1]);
        upper_z = std::max(upper_z, t.verts[i][2]);
    }
}

void triangle_intersect_func(const RTCIntersectFunctionNArguments *args)
{
    RTCRay *ray = (RTCRay *)args->rayhit;

    void *ptr = args->geometryUserPtr;
    const std::vector<stl::Triangle> &tri_soup = ((Data *)ptr)->tri_soup;
    const stl::Triangle &tri = tri_soup[args->primID];

    Vec3 *verts = (Vec3 *)tri.verts;

    auto &v0 = verts[0];
    auto &v1 = verts[1];
    auto &v2 = verts[2];
    auto e1 = v0 - v1;
    auto e2 = v2 - v0;
    auto normal = e1.cross(e2);

    /* calculate denominator */
    auto O = Vec3(ray->org_x, ray->org_y, ray->org_z);
    auto D = Vec3(ray->dir_x, ray->dir_y, ray->dir_z);
    auto C = v0 - O;
    auto R = D.cross(C);
    float den = normal.dot(D);
    float rcpDen = 1.0f / den;

    /* perform edge tests */
    float u = R.dot(e2) * rcpDen;
    float v = R.dot(e1) * rcpDen;

    /* perform backface culling */
    // "likely" would be used here to hint the branch predictor
    // If the use case is rendeing an image, then we use likely, as we assume most triangles
    // are tiny on screen
    // in other words it is likely for the ray to miss the triangle when rendering an image
    // however, if the use case is collision detection
    // then the BVH will yield to us the pairs of triangles that are likely to intersect
    // and thus we use likely(valid) or unlikely(!valid) in collision detection use case
    bool valid = (den != 0.0f) & (u >= 0.0f) & (v >= 0.0f) & (u + v <= 1.0f);
    if (likely(!valid))
        return;

    /* perform depth test */
    float t = normal.dot(C) * rcpDen;
    valid &= (t > ray->tnear) & (t < ray->tfar);
    if (likely(!valid))
        return;

    /* update hit */
    ray->tfar = t;
}
