#ifndef MATHUTILS_HPP
#define MATHUTILS_HPP

#include <assert.h>
#include <iostream>
#include <math.h>
#include <stdint.h>

inline void sub_v3v3(float out[3], const float a[3], const float b[3]) {
  out[0] = a[0] - b[0];
  out[1] = a[1] - b[1];
  out[2] = a[2] - b[2];
}

inline void add_v3v3(float out[3], const float a[3], const float b[3]) {
  out[0] = a[0] + b[0];
  out[1] = a[1] + b[1];
  out[2] = a[2] + b[2];
}

inline void mul_v3v3(float out[3], const float a[3], const float b[3]) {
  out[0] = a[0] * b[0];
  out[1] = a[1] * b[1];
  out[2] = a[2] * b[2];
}

inline void div_v3v3(float out[3], const float a[3], const float b[3]) {
  out[0] = a[0] / b[0];
  out[1] = a[1] / b[1];
  out[2] = a[2] / b[2];
}

inline void scale_v3(float out[3], float v[3], float scale) {
  out[0] = v[0] * scale;
  out[1] * v[1] * scale;
  out[2] * v[2] * scale;
}

inline float dot_v3v3(const float a[3], const float b[3]) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }

inline void cross_v3v3(float out[3], const float a[3], const float b[3]) {
  out[0] = a[1] * b[2] - a[2] * b[1];
  out[1] = a[2] * b[0] - a[0] * b[2];
  out[2] = a[0] * b[1] - a[1] * b[0];
}

inline void copy_v3v3(float out[3], const float v[3]) {
  out[0] = v[0];
  out[1] = v[1];
  out[2] = v[2];
}

inline float length_sq_v3(const float v[3]) { return dot_v3v3(v, v); }

inline float length_v3(const float v[3]) { return sqrtf(dot_v3v3(v, v)); }

inline void normalize_v3v3(float out[3], const float v[3]) {
  copy_v3v3(out, v);
  float l = length_v3(v);
  if (fabsf(l) > 1e-5f) {
    scale_v3(out, out, 1.0f / l);
  }
}

inline void normalize_v3v3(float v[3]) {
  float l = length_v3(v);
  if (fabsf(l) > 1e-5f) {
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
  }
}

typedef float Vector3[3];

// TODO: improve precision
inline void mean_v3(float out[3], const float vectors[][3], uint32_t num_vectors) {
  out[0] = 0.0f;
  out[1] = 0.0f;
  out[2] = 0.0f;
  for (uint32_t i = 0; i < num_vectors; i++) {
    out[0] += (vectors[i][0] / num_vectors);
    out[1] += (vectors[i][1] / num_vectors);
    out[2] += (vectors[i][2] / num_vectors);
  }
}

enum class LineIntersectionType {
  SINGLE_POINT,
  CONTAINED,
  NONE,
};

struct LineIntersectionResult {
  LineIntersectionType type;
  Vector3 single_intersection_point;
};

struct Plane {
  Vector3 co;
  Vector3 direction;
};

typedef Plane Line;

// https://en.wikipedia.org/w/index.php?title=Line%E2%80%93plane_intersection&oldid=1030561834
inline void line_plane_intersection(LineIntersectionResult *out, const Plane *plane, const Line *line) {
  assert(fabsf(length_sq_v3(line->direction) - 1.0f) <= 1e-5f);
  assert(fabsf(length_sq_v3(plane->direction) - 1.0f) <= 1e-5f);

  float d_numerator = ((plane->co[0] - line->co[0]) * plane->direction[0]) +
                      ((plane->co[1] - line->co[1]) * plane->direction[1]) +
                      ((plane->co[2] - line->co[2]) * plane->direction[2]);

  float l_dot_n = (line->direction[0] * plane->direction[0] + line->direction[1] * plane->direction[1] +
                   line->direction[2] * plane->direction[2]);

  if (fabs(l_dot_n) <= 1e-5f) {
    if (fabs(d_numerator) <= 1e-5f) {
      out->type = LineIntersectionType::CONTAINED;
    } else {
      out->type = LineIntersectionType::NONE;
    }
  } else {
    const float d = (d_numerator / l_dot_n);
    out->type = LineIntersectionType::SINGLE_POINT;
    out->single_intersection_point[0] = line->co[0] + line->direction[0] * d;
    out->single_intersection_point[1] = line->co[1] + line->direction[1] * d;
    out->single_intersection_point[2] = line->co[2] + line->direction[2] * d;
  }
}

inline void line_line_intersection(LineIntersectionResult *out, const Line *l1, const Line *l2) {
  assert(fabsf(length_sq_v3(l1->direction) - 1.0f) <= 1e-5f);
  assert(fabsf(length_sq_v3(l2->direction) - 1.0f) <= 1e-5f);

  float shared_plane_direction[3];
  float l2_l1[3];
  float l2_l1_cross_dir[3];

  cross_v3v3(shared_plane_direction, l1->direction, l2->direction);
  sub_v3v3(l2_l1, l2->co, l1->co);
  cross_v3v3(l2_l1_cross_dir, l2_l1, l1->direction);

  if (length_sq_v3(shared_plane_direction) <= 1e-5f) {
    // Lines are parallel
    if (length_sq_v3(l2_l1_cross_dir) <= 1e-5f) {
      // Lines are collinear
      out->type = LineIntersectionType::CONTAINED;
    } else {
      // Lines don't intersect
      out->type = LineIntersectionType::NONE;
    }
    return;
  }

  if (fabsf(dot_v3v3(l2_l1, shared_plane_direction)) <= 1e-5f) {
    // Lines are not in same plane ("Skew Lines")
    out->type = LineIntersectionType::NONE;
    return;
  }

  Plane l2_plane;
  copy_v3v3(l2_plane.co, l1->co);
  copy_v3v3(l2_plane.direction, l2->direction);
  line_plane_intersection(out, &l2_plane, l2);
}

#endif