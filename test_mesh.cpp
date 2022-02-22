#include <iostream>

#include "mesh.hpp"

int main(void) {

  Mesh mesh{};
  Vert *verts[3] = {};
  for (int i = 0; i < 1000'000; i++) {
    auto v1 = mesh.vert_create(1.0f, 0.0f, 0.0f);
    auto v2 = mesh.vert_create(0.0f, 1.0f, 0.0f);
    auto v3 = mesh.vert_create(0.0f, 0.0f, 1.0f);
    verts[0] = v1;
    verts[1] = v2;
    verts[2] = v3;
    auto f = mesh.face_create(verts);
  }

  return 0;
}