#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "mesh.h"

Vert *create_vertex(Mesh *mesh, float location[3]) {
  Vert *new_vert = malloc(sizeof(Vert));
  if (!new_vert) {
    return NULL;
  }
  memcpy(new_vert->location, location, sizeof(float[3]));
  prepend(&(mesh->vertices), new_vert);
  return new_vert;
}

/* TODO: ensure unique edges */
Edge *create_edge(Mesh *mesh, Vert *v1, Vert *v2) {
  Edge *new_edge = malloc(sizeof(Edge));
  if (!new_edge) {
    return NULL;
  }
  new_edge->v1 = v1;
  new_edge->v2 = v2;

  prepend(&(mesh->edges), new_edge);
  prepend(&(v1->link_edges), new_edge);
  prepend(&(v2->link_edges), new_edge);
}

/* TODO: support n-gons */
Face *create_face(Mesh *mesh, Vert *v1, Vert *v2, Vert *v3) {
  Face *new_face;
  Edge *e1, *e2, *e3;

  new_face = malloc(sizeof(Face));
  if (!new_face) {
    return NULL;
  }

  e1 = create_edge(mesh, v1, v2);
  if (!e1) {
    return NULL;
  }
  e2 = create_edge(mesh, v2, v3);
  if (!e2) {
    return NULL;
  }
  e3 = create_edge(mesh, v3, v1);
  if (!e3) {
    return NULL;
  }

  prepend(&(mesh->faces), new_face);

  prepend(&(v1->link_faces), new_face);
  prepend(&(v2->link_faces), new_face);
  prepend(&(v3->link_faces), new_face);

  prepend(&(e1->link_faces), new_face);
  prepend(&(e2->link_faces), new_face);
  prepend(&(e3->link_faces), new_face);
}