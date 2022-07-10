#pragma once

#include <embree3/rtcore.h>
// #include <embree3/rtcore_ray.h>
#include <vector>

#include "stl_io.hh"
#include "vec3.hh"

using namespace mp::io;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define likely(expr) (expr)
#define unlikely(expr) (expr)
#else
#define likely(expr) __builtin_expect((bool)(expr), true)
#define unlikely(expr) __builtin_expect((bool)(expr), false)
#endif

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
