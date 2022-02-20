#include <iostream>

#include "mesh.hpp"

int main(void) {

  Mesh mesh{};
  auto v1 = mesh.vert_create(1.0f, 0.0f, 0.0f);
  auto v2 = mesh.vert_create(0.0f, 1.0f, 0.0f);
  auto v3 = mesh.vert_create(0.0f, 0.0f, 1.0f);
  uint32_t verts[3] = {v1, v2, v3};
  auto f = mesh.face_create(verts);

  for (int i = 0; i < 1000; i++) {
    mesh.vert_create(1.0f, 0.0f, 0.0f);
  }

  mesh.vert_remove(v1);
  std::cout << mesh.faces.size() << std::endl;
  return 0;
}