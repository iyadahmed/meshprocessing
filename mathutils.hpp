#ifndef MATHUTILS_HPP
#define MATHUTILS_HPP

#include <stdint.h>

enum class LinePlaneIntersectionType {
  SINGLE_POINT,
  CONTAINED,
  NONE,
};

struct LinePlaneIntersectionResult {
  LinePlaneIntersectionType type;
  float single_intersection_point[3];
};

// float *covariance(float arr[], uint32_t num_elems);// TODO
void line_plane_intersection(LinePlaneIntersectionResult *out, const float plane_co[3], const float plane_normal[3],
                             const float line_co[3], const float line_direction[3]);
#endif