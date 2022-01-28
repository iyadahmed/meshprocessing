#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "mesh.h"

int main(int argc, char **argv) {
  Mesh *mesh = malloc(sizeof(Mesh));
  if (!mesh) {
    puts("Failed to allocate memory for Mesh");
    return 1;
  }
  mesh->vertices = NULL;
  mesh->edges = NULL;
  mesh->faces = NULL;

  Vert *v1 = create_vertex(mesh, (float[3]){0.0, 1.0, 0.0});
  Vert *v2 = create_vertex(mesh, (float[3]){-1.0, 0.0, 0.0});
  Vert *v3 = create_vertex(mesh, (float[3]){1.0, 0.0, 0.0});
  Face *f1 = create_face(mesh, v1, v2, v3, NULL);

  /* FIXME: infinite loop if face is duplicate */
  bool already_exists = false;
  // Face *f2 = create_face(mesh, v1, v2, v3, &already_exists);

  if (already_exists) {
    puts("Duplicate faces");
  }
  // printf("Face1: %p\nFace2: %p\n", f1, f2);
  return 0;
}