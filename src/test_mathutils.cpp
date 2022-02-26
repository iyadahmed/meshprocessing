#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>

#include "mathutils.hpp"

int main(void) {
  float pco[3] = {1.0f, 1.0f, 1.0f};
  float pno[3] = {1.0f, 1.0f, 1.0f};
  float lco[3] = {0.0f, 0.0f, 0.0f};
  float ldir[3] = {1.0f, 1.0f, 1.0f};
  LinePlaneIntersectionResult res{LinePlaneIntersectionType::NONE, {0.0f}};

  for (uint32_t i = 0; i < 1000'000'0; i++) {
    line_plane_intersection(&res, pco, pno, lco, ldir);
  }

  assert(res.type == LinePlaneIntersectionType::SINGLE_POINT);
  assert(res.single_intersection_point[0] == 1.0f);
  assert(res.single_intersection_point[1] == 1.0f);
  assert(res.single_intersection_point[2] == 1.0f);
  return 0;
}