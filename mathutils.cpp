#include "math.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "mathutils.hpp"

// TODO
// void covariance3(float out[3][3], float arr[][3], uint32_t num_elems) {
//   float mean[3] = {0.0f};
//   float cov = 0.0f;
//   for (uint32_t i = 0; i < num_elems; i++) {
//     mean[0] += arr[i][0] / num_elems;
//     mean[1] += arr[i][1] / num_elems;
//     mean[2] += arr[i][2] / num_elems;
//   }
//   for (uint32_t i = 0; i < num_elems; i++) {
//     cov += (arr[i][0] - mean[0]) * (arr[i][1] - mean[1]) * (arr[i][2] - mean[2]) / num_elems;
//   }
// }



// https://en.wikipedia.org/w/index.php?title=Line%E2%80%93plane_intersection&oldid=1030561834
void line_plane_intersection(LinePlaneIntersectionResult *out, const float plane_co[3], const float plane_normal[3],
                             const float line_co[3], const float line_direction[3]) {

  float d_numerator = ((plane_co[0] - line_co[0]) * plane_normal[0]) + ((plane_co[1] - line_co[1]) * plane_normal[1]) +
                      ((plane_co[2] - line_co[2]) * plane_normal[2]);

  float l_dot_n =
      (line_direction[0] * plane_normal[0] + line_direction[1] * plane_normal[1] + line_direction[2] * plane_normal[2]);

  if (fabs(l_dot_n) <= 1e-5f) {
    if (fabs(d_numerator) <= 1e-5f) {
      out->type = LinePlaneIntersectionType::CONTAINED;
    } else {
      out->type = LinePlaneIntersectionType::NONE;
    }
  } else {
    const float d = (d_numerator / l_dot_n);
    out->type = LinePlaneIntersectionType::SINGLE_POINT;
    out->single_intersection_point[0] = line_co[0] + line_direction[0] * d;
    out->single_intersection_point[1] = line_co[1] + line_direction[1] * d;
    out->single_intersection_point[2] = line_co[2] + line_direction[2] * d;
  }
}