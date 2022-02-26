#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>

#include "mathutils.hpp"

void test_line_plane_intersection() {
  LinePlaneIntersectionResult res{LinePlaneIntersectionType::NONE, {0.0f}};
  Plane plane{{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}};
  Line line{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};

  for (uint32_t i = 0; i < 1000'000'0; i++) {
    line_plane_intersection(&res, &plane, &line);
  }

  assert(res.type == LinePlaneIntersectionType::SINGLE_POINT);
  assert(res.single_intersection_point[0] == 1.0f);
  assert(res.single_intersection_point[1] == 1.0f);
  assert(res.single_intersection_point[2] == 1.0f);
}

void test_cross_product() {
  float out[3]{};
  float a[3] = {1.0f, 0.0f, 0.0f};
  float b[3] = {0.0f, 1.0f, 0.0};
  cross_v3v3(out, a, b);
  assert(fabs(out[0]) <= 1e-5f);
  assert(fabs(out[1]) <= 1e-5f);
  assert(fabs(out[2] - 1.0f) <= 1e-5f);
}

int main() {
  test_line_plane_intersection();
  test_cross_product();
  return 0;
}