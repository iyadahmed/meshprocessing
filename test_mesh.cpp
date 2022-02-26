#include <iostream>

#include "mesh.hpp"

int main(void) {

  Mesh mesh{};

  float v1[3] = {1.0f, 0.0f, 0.0f};
  float v2[3] = {0.0f, 1.0f, 0.0f};
  float v3[3] = {0.0f, 0.0f, 1.0f};
  auto tri = Triangle{};

  for (int i = 0; i < 1000'000; i++) {
    mesh.triangles.push_back(tri);
  }

  return 0;
  }