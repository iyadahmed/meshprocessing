#include <cmath>
#include <cstdio>

#include "stl.hpp"

int main(int argc, char **argv) {
  if (argc < 2) {
    puts("Usage: test_stl path_to_stl.stl");
    return 1;
  }

  Mesh mesh{};

  read_stl(mesh, argv[1]);

  float bb_min[3] = {INFINITY, INFINITY, INFINITY};
  float bb_max[3] = {-INFINITY, -INFINITY, -INFINITY};

  for (auto t : mesh.triangles) {
    for (int j = 0; j < 3; j++) {
      float *v = t.vertices[j];
      bb_min[j] = fminf(v[j], bb_min[j]);
      bb_max[j] = fmaxf(v[j], bb_max[j]);
    }
  }

  printf("Number of triangles: %lu\n", mesh.triangles.size());
  printf("Bounding Box: Max(%f, %f, %f), Min(%f, %f, %f)\n", bb_max[0], bb_max[1], bb_max[2], bb_min[0], bb_min[1],
         bb_min[2]);

  return 0;
}