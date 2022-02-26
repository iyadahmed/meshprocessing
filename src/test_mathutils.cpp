#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>

#include "mathutils.hpp"

// Use real data instead of pointless iteration, as compiler might optimize away the iteration

void test_line_plane_intersection() {
  LineIntersectionResult res{LineIntersectionType::NONE, {0.0f, 0.0f, 0.0f}};
  float plane_normal[3] = {1.0f, 1.0f, 1.0f};
  normalize_v3v3(plane_normal);

  Plane plane{{1.0f, 1.0f, 1.0f}, {plane_normal[0], plane_normal[1], plane_normal[2]}};
  Line line{{0.0f, 0.0f, 0.0f}, {plane_normal[0], plane_normal[1], plane_normal[2]}};

  for (uint32_t i = 0; i < 10'000'000; i++) {
    line_plane_intersection(&res, &plane, &line);
  }

  assert(res.type == LineIntersectionType::SINGLE_POINT);
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

void test_line_line_intersect() {
  LineIntersectionResult res{LineIntersectionType::NONE, {0.0f, 0.0f, 0.0f}};
  Line l1{{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};
  Line l2{{-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};

  for (uint32_t i = 0; i < 10'000'00; i++) {
    line_line_intersection(&res, &l1, &l2);
  }
}

int main() {
  test_line_plane_intersection();
  test_cross_product();
  test_line_line_intersect();
  return 0;
}