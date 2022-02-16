#include <cmath>
#include <cstdio>

#include "stl.hpp"

int main(int argc, char **argv) {
  if (argc < 2) {
    puts("Usage: test_stl path_to_stl.stl");
    return 1;
  }
  auto triangle_buffer = read_stl(argv[1]);
  if (triangle_buffer == NULL) {
    puts("Failed to read STL");
    return 1;
  }

  float bb_min[3] = {INFINITY};
  float bb_max[3] = {-INFINITY};

  for (int i = 0; i < triangle_buffer->num_appended_items; i++) {
    auto tri = triangle_buffer->mem_start[i];
    for (int j = 0; j < 3; j++) {
      float *v = tri.vertices[j];
      bb_min[j] = fminf(v[j], bb_min[j]);
      bb_max[j] = fmaxf(v[j], bb_max[j]);
    }
  }

  printf("Number of triangles: %u\n", triangle_buffer->num_appended_items);
  printf("Bounding Box: Max(%f, %f, %f), Min(%f, %f, %f)\n", bb_max[0], bb_max[1], bb_max[2], bb_min[0], bb_min[1],
         bb_min[2]);

  delete triangle_buffer;
  return 0;
}