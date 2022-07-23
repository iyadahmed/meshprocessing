#pragma once

#include <embree3/rtcore.h>
#include <limits>

bool do_intersect(const RTCScene &scene, float ox, float oy, float oz, float dx,
                  float dy, float dz) {
  /*
   * The intersect context can be used to set intersection
   * filters or flags, and it also contains the instance ID stack
   * used in multi-level instancing.
   */
  struct RTCIntersectContext context;
  rtcInitIntersectContext(&context);

  struct RTCRay ray;
  ray.org_x = ox;
  ray.org_y = oy;
  ray.org_z = oz;
  ray.dir_x = dx;
  ray.dir_y = dy;
  ray.dir_z = dz;
  ray.tnear = 0;
  ray.tfar = std::numeric_limits<float>::infinity();
  ray.mask = -1;
  ray.flags = 0;

  /*
   * When no intersection is found,
   * the ray data is not updated.
   * In case a hit was found,
   * the tfar component of the ray is set to -inf.
   */
  rtcOccluded1(scene, &context, &ray);

  return ray.tfar == -std::numeric_limits<float>::infinity();
}
