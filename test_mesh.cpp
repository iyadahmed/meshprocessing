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
  auto f1 = create_face_tri(mesh, v1, v2, v3);
  auto f2 = create_face_tri(mesh, v1, v2, v3);

  printf("Face1: %p\nFace2: %p\n", &(*f1), &(*f2));

  face_remove(mesh, f1);

  delete mesh;
  return 0;
}