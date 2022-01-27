#include "stl.h"
#include <stdio.h>

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
  size_t num_tri = 0;
  while (tri_list) {
    tri_list = tri_list->next;
    num_tri++;
  }
  printf("Number of triangles: %lu\n", num_tri);
  return 0;
}