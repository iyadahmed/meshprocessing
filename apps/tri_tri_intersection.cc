#include "vec3.hh"

struct Triangle
{
    Vec3 a, b, c;
    Vec3 calc_normal() { return (a - b).cross(c); }
};

struct Plane
{
    Vec3 normal;
    Vec3 point;
};

Vec3 dot_tri_plane(Triangle &t, Plane &p)
{
    return Vec3((t.a - p.point).dot(p.normal), (t.b - p.point).dot(p.normal),
                (t.c - p.point).dot(p.normal));
}

void tri_tri_intersection(Triangle t1, Triangle t2)
{
    auto p1 = Plane{t1.calc_normal(), t1.a};
    auto p2 = Plane{t2.calc_normal(), t2.a};

    auto d1 = dot_tri_plane(t1, p2);
    auto d2 = dot_tri_plane(t2, p1);

    // Max number of intersections per triangle-plane is 6
    // as each edge
    Vec3 t1p1[6];

    // Triangle 1
    // Edge 1
    if (fabsf(d1.x) <= 1e-8 && fabsf(d1.y) <= 1e-8)
    {
    }
}