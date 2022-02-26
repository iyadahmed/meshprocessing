#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <stdint.h>

// float *covariance(float arr[], uint32_t num_elems);// TODO
void line_plane_intersection(LinePlaneIntersectionResult *out, const float plane_co[3], const float plane_normal[3],
                             const float line_co[3], const float line_direction[3]);
#endif