#ifndef MESH_H
#define MESH_H

#include <stdbool.h>

#include "list.h"

typedef List MeshVertItem;
typedef List MeshEdgeItem;
typedef List MeshFaceItem;

typedef List MeshVertItemList;
typedef List MeshEdgeItemList;
typedef List MeshFaceItemList;

typedef List LoopList;

typedef struct Vert {
  float location[3];
  MeshEdgeItemList *link_edge_items;
  MeshFaceItemList *link_face_items;
} Vert;

typedef struct Edge {
  MeshVertItem *vert_item1;
  MeshVertItem *vert_item2;
  MeshFaceItemList *link_face_items;
} Edge;

typedef struct Loop {
  MeshVertItem *vert_item;
  MeshEdgeItem *edge_item;
} Loop;

typedef struct Face {
  float normal[3];
  LoopList *loops;
} Face;

typedef struct Mesh {
  MeshVertItem *vert_items;
  MeshVertItem *edge_items;
  MeshVertItem *face_items;
} Mesh;

MeshVertItem *create_vertex(Mesh *mesh, float location[3]);
MeshEdgeItem *create_edge(Mesh *mesh, MeshVertItem *v1, MeshVertItem *v2,
                          bool *already_exists);
MeshFaceItem *create_face(Mesh *mesh, MeshVertItem *v1, MeshVertItem *v2,
                          MeshVertItem *v3, bool *already_exists);

void vert_remove(MeshVertItem *vert_item);
void face_remove_keep_edges_verts(MeshFaceItem *face_item);
void edge_remove_keep_verts(MeshEdgeItem *edge_item);

void mesh_free(Mesh **mesh_ref);

#endif