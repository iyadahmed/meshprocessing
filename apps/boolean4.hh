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
typedef K::Triangle_3 Triangle;

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

/* Self-intersection data */
struct Data
{
    std::vector<stl::Triangle> tri_soup;
};

inline Triangle to_cgal_triangle(const stl::Triangle &t)
{
    return {
        {t.v1[0], t.v1[1], t.v1[2]},
        {t.v2[0], t.v2[1], t.v2[2]},
        {t.v3[0], t.v3[1], t.v3[2]},
    };
}

inline bool intersect_triangle_triangle(const std::vector<stl::Triangle> &tri_soup, unsigned geomID0, unsigned primID0, unsigned geomID1, unsigned primID1)
{
    if (primID0 == primID1)
    {
        return false;
    }

    const stl::Triangle &t1 = tri_soup[primID0];
    const stl::Triangle &t2 = tri_soup[primID1];
    return CGAL::do_intersect(to_cgal_triangle(t1), to_cgal_triangle(t2));
}

inline void collide_func(void *user_data_ptr, RTCCollision *collisions, unsigned int num_collisions)
{
    for (size_t i = 0; i < num_collisions;)
    {
        bool intersect = intersect_triangle_triangle(((Data *)user_data_ptr)->tri_soup,
                                                     collisions[i].geomID0, collisions[i].primID0,
                                                     collisions[i].geomID1, collisions[i].primID1);
        if (intersect)
            i++;
        else
            collisions[i] = collisions[--num_collisions];
    }

    if (num_collisions == 0)
        return;

    // TODO: collect intersections
}

void triangle_bounds_func(const struct RTCBoundsFunctionArguments *args)
{
    void *ptr = args->geometryUserPtr;
    const std::vector<stl::Triangle> &tri_soup = ((Data *)ptr)->tri_soup;
    const stl::Triangle &t = tri_soup[args->primID];

    BBox3fa bounds{};
    for (int i = 0; i < 3; i++)
    {
        bounds.extend(t.verts[i]);
    }

    args->bounds_o->lower_x = bounds.lower.x;
    args->bounds_o->lower_y = bounds.lower.y;
    args->bounds_o->lower_z = bounds.lower.z;

    args->bounds_o->upper_x = bounds.upper.x;
    args->bounds_o->upper_y = bounds.upper.y;
    args->bounds_o->upper_z = bounds.upper.z;
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
    auto Ng = e1.cross(e2);

    /* calculate denominator */
    auto O = Vec3(ray->org_x, ray->org_y, ray->org_z);
    auto D = Vec3(ray->dir_x, ray->dir_y, ray->dir_z);
    auto C = v0 - O;
    auto R = D.cross(C);
    float den = Ng.dot(D);
    float rcpDen = 1.0f / den;

    /* perform edge tests */
    float u = R.dot(e2) * rcpDen;
    float v = R.dot(e1) * rcpDen;

    /* perform backface culling */
    bool valid = (den != 0.0f) & (u >= 0.0f) & (v >= 0.0f) & (u + v <= 1.0f);
    if (likely(!valid))
        return;

    /* perform depth test */
    float t = Ng.dot(C) * rcpDen;
    valid &= (t > ray->tnear) & (t < ray->tfar);
    if (likely(!valid))
        return;

    /* update hit */
    ray->tfar = t;
}
