#ifndef MESH_H
#define MESH_H

#include <stdbool.h>

#include "list.h"

typedef List VertList;
typedef List EdgeList;
typedef List FaceList;

typedef struct Vert {
  float location[3];

  EdgeList *link_edges;
  FaceList *link_faces;
} Vert;

typedef struct Edge {
  Vert *v1;
  Vert *v2;
  FaceList *link_faces;
} Edge;

typedef struct Loop {
  Vert *vert;
  Edge *edge;

  struct Loop *next;
} Loop;

typedef struct Face {
  float normal[3];

  Loop *loop_first;
} Face;

typedef struct Mesh {
  VertList *vertices;
  EdgeList *edges;
  FaceList *faces;
} Mesh;

Vert *create_vertex(Mesh *mesh, float location[3]);
Edge *create_edge(Mesh *mesh, Vert *v1, Vert *v2, bool *already_exists);
Face *create_face(Mesh *mesh, Vert *v1, Vert *v2, Vert *v3,
                  bool *already_exists);

void mesh_free(Mesh **mesh_ref);

#endif