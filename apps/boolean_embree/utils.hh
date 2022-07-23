#pragma once

#include "vec3.hh"

int plane_side(Vec3 point, Vec3 plane_co, Vec3 plane_normal, float tolerance) {
  Vec3 vec = point - plane_co;
  float d = vec.dot(plane_normal);
  if (std::abs(d) <= tolerance) {
    return true;
  }
  return (d > 0.0f) ? 1 : -1;
}

Vec3 closest_point_to_plane(Vec3 point, Vec3 plane_co, Vec3 plane_normal) {
  return point - (point - plane_co).dot(plane_normal) * plane_normal;
}

// https://stackoverflow.com/a/23976134/8094047
Vec3 ray_plane_intersection_unchecked(Vec3 ro, Vec3 rd, Vec3 plane_co,
                                      Vec3 plane_normal) {
  float denom = plane_normal.dot(rd);
  float t = (plane_co - ro).dot(plane_normal) / denom;
  return ro + t * rd;
}

bool is_degenerate(Vec3 a, Vec3 b, Vec3 c, float tolerance) {
  return (b - a).cross(c - a).length_squared() < (tolerance * tolerance);
}

// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
Vec3 barycentric(const Vec3 &p, const Vec3 &a, const Vec3 &b, const Vec3 &c) {
  Vec3 v0 = b - a; // NOTE: can be computed once per triangle
  Vec3 v1 = c - a; // NOTE: can be computed once per triangle
  Vec3 v2 = p - a;
  float d00 = dot(v0, v0);
  float d01 = dot(v0, v1);
  float d11 = dot(v1, v1);
  float d20 = dot(v2, v0);
  float d21 = dot(v2, v1);
  float denom = d00 * d11 - d01 * d01;
  float v = (d11 * d20 - d01 * d21) / denom;
  float w = (d00 * d21 - d01 * d20) / denom;
  float u = 1.0f - v - w;
  return {u, v, w};
}

bool is_inside(const Vec3 &p, const Vec3 &a, const Vec3 &b, const Vec3 &c,
               float tolerance) {
  auto [u, v, w] = barycentric(p, a, b, c);
  float upper = 1.0f + tolerance;
  float lower = 0.0f - tolerance;
  return (u >= lower) && (u <= upper) && (v >= lower) && (v <= upper) &&
         (w >= lower) && (w <= upper);
}
