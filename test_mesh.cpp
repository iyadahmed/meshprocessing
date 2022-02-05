#include "iostream"

#include "mesh.hpp"

int main(int argc, char **argv) {

  Mesh *mesh = new Mesh;

  float p1[3] = {0.0, 1.0, 0.0};
  float p2[3] = {-1.0, 0.0, 0.0};
  float p3[3] = {1.0, 0.0, 0.0};

  auto v1 = create_vertex(mesh, p1);
  auto v2 = create_vertex(mesh, p2);
  auto v3 = create_vertex(mesh, p3);
  auto f1 = create_face(mesh, v1, v2, v3, nullptr);

  bool already_exists = false;
  auto f2 = create_face(mesh, v1, v2, v3, &already_exists);

  /* FIXME: duplicate faces are not detected */
  if (already_exists) {
    std::cout << "Duplicate faces" << std::endl;
  }

  printf("Face1: %p\nFace2: %p\n", &(*f1), &(*f2));

  delete mesh;
  return 0;
}