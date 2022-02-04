#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "mesh.h"

int main(int argc, char **argv) {
  Mesh *mesh = malloc(sizeof(Mesh));
  if (NULL == mesh) {
    puts("Failed to allocate memory for Mesh");
    return 1;
  }
  mesh->vert_items = NULL;
  mesh->edge_items = NULL;
  mesh->face_items = NULL;

  MeshVertItem *v1 = create_vertex(mesh, (float[3]){0.0, 1.0, 0.0});
  MeshVertItem *v2 = create_vertex(mesh, (float[3]){-1.0, 0.0, 0.0});
  MeshVertItem *v3 = create_vertex(mesh, (float[3]){1.0, 0.0, 0.0});
  MeshFaceItem *f1 = create_face(mesh, v1, v2, v3, NULL);

  bool already_exists = false;
  MeshFaceItem *f2 = create_face(mesh, v1, v2, v3, &already_exists);

  if (already_exists) {
    puts("Duplicate faces");
  }
  printf("Face1: %p\nFace2: %p\n", f1, f2);

  mesh_free(&mesh);
  return 0;
}