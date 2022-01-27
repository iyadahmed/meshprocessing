#include <math.h>
#include <stdio.h>

#include "stl.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    puts("Usage: example path_to_stl.stl");
    return 1;
  }
  TriangleList *tri_list = read_stl(argv[1]);
  if (!tri_list) {
    puts("Failed to read STL");
    return 1;
  }
  float bb_min[3] = {INFINITY};
  float bb_max[3] = {-INFINITY};
  size_t num_tri = 0;
  while (tri_list) {
    for (int i = 0; i < 3; i++) {
      float *v = tri_list->data->vertices[i];
      for (int j = 0; j < 3; j++) {
        bb_min[j] = fminf(v[j], bb_min[j]);
        bb_max[j] = fmaxf(v[j], bb_max[j]);
      }
    }
    tri_list = tri_list->next;
    num_tri++;
  }
  printf("Number of triangles: %lu\n", num_tri);
  printf("Bounding Box: Max(%f, %f, %f), Min(%f, %f, %f)\n", bb_max[0],
         bb_max[1], bb_max[2], bb_min[0], bb_min[1], bb_min[2]);

  free_triangles(&tri_list);
  return 0;
}