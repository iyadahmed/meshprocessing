#ifndef MESH_H
#define MESH_H

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

typedef struct Face {
  float normal[3];

  VertList vertices;
  EdgeList *edges;
} Face;

typedef struct Mesh {
  VertList *vertices;
  EdgeList *edges;
  FaceList *faces;
} Mesh;

#endif