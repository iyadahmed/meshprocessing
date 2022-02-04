#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "mesh.h"

#define VERT_FROM_VERT_ITEM(item) ((Vert *)(item->data))
#define EDGE_FROM_EDGE_ITEM(item) ((Edge *)(item->data))
#define FACE_FROM_FACE_ITEM(item) ((Face *)(item->data))

static void loop_list_prepend(LoopList **head_ref, MeshVertItem *vert_item,
                              MeshEdgeItem *edge_item) {
  Loop *loop = malloc(sizeof(Loop));
  if (NULL == loop) {
    return;
  }
  loop->vert_item = vert_item;
  loop->edge_item = edge_item;
  list_prepend(head_ref, loop);
}

MeshVertItem *create_vertex(Mesh *mesh, float location[3]) {
  Vert *new_vert = malloc(sizeof(Vert));
  if (NULL == new_vert) {
    return NULL;
  }
  new_vert->link_edge_items = NULL;
  new_vert->link_face_items = NULL;
  memcpy(new_vert->location, location, sizeof(float[3]));
  return list_prepend(&(mesh->vert_items), new_vert);
}

MeshEdgeItem *create_edge(Mesh *mesh, MeshVertItem *v1, MeshVertItem *v2,
                          bool *already_exists) {
  MeshEdgeItemList *link_edge_iter = VERT_FROM_VERT_ITEM(v1)->link_edge_items;
  MeshEdgeItem *link_edge_item = NULL;
  Edge *link_edge = NULL;
  while (link_edge_iter) {
    link_edge_item = link_edge_iter->data;
    link_edge = EDGE_FROM_EDGE_ITEM(link_edge_item);
    if (((link_edge->vert_item1 == v1) && (link_edge->vert_item2 == v2)) ||
        ((link_edge->vert_item1 == v2) && (link_edge->vert_item2 == v1))) {
      if (already_exists) {
        *already_exists = true;
      }
      return link_edge_item;
    }
    link_edge_iter = link_edge_iter->next;
  }

  Edge *new_edge = malloc(sizeof(Edge));
  if (NULL == new_edge) {
    return NULL;
  }
  new_edge->link_face_items = NULL;
  new_edge->vert_item1 = v1;
  new_edge->vert_item2 = v2;

  MeshEdgeItem *new_edge_item = list_prepend(&(mesh->edge_items), new_edge);

  list_prepend(&(VERT_FROM_VERT_ITEM(v1)->link_edge_items), new_edge_item);
  list_prepend(&(VERT_FROM_VERT_ITEM(v2)->link_edge_items), new_edge_item);

  if (already_exists) {
    *already_exists = false;
  }
  return new_edge_item;
}

/* TODO: function for creating n-gons */
MeshFaceItem *create_face(Mesh *mesh, MeshVertItem *v1, MeshVertItem *v2,
                          MeshVertItem *v3, bool *already_exists) {
  Face *new_face = NULL;
  MeshEdgeItem *e1 = NULL, *e2 = NULL, *e3 = NULL;
  bool edge_already_exists[3] = {false};

  e1 = create_edge(mesh, v1, v2, edge_already_exists);
  if (NULL == e1) {
    return NULL;
  }
  e2 = create_edge(mesh, v2, v3, edge_already_exists + 1);
  if (NULL == e2) {
    return NULL;
  }
  e3 = create_edge(mesh, v3, v1, edge_already_exists + 2);
  if (NULL == e3) {
    return NULL;
  }

  /* Check if face already exists, return it if so */
  MeshEdgeItemList *link_faces_iter = VERT_FROM_VERT_ITEM(v1)->link_face_items;
  MeshFaceItem *link_face_item = NULL;
  LoopList *link_face_loops_iter = NULL;
  MeshEdgeItem *link_face_edge_item = NULL;
  size_t num_edges_link_face = 0;
  bool edge_found[3] = {false};

  if (edge_already_exists[0] && edge_already_exists[1] &&
      edge_already_exists[2]) {
    while (link_faces_iter) {
      link_face_item = link_faces_iter->data;

      link_face_loops_iter = FACE_FROM_FACE_ITEM(link_face_item)->loops;

      num_edges_link_face = 0;
      memset(edge_found, false, 3);
      while (link_face_loops_iter) {
        link_face_edge_item = ((Loop *)(link_face_loops_iter->data))->edge_item;

        if (link_face_edge_item == e1) {
          edge_found[0] = true;
        } else if (link_face_edge_item == e2) {
          edge_found[1] = true;
        } else if (link_face_edge_item == e3) {
          edge_found[2] = true;
        }
        link_face_loops_iter = link_face_loops_iter->next;
        num_edges_link_face++;
      }

      if (edge_found[0] && edge_found[1] && edge_found[2] &&
          (num_edges_link_face == 3)) {
        if (already_exists) {
          *already_exists = true;
        }
        return link_face_item;
      }

      link_faces_iter = link_faces_iter->next;
    }
  }

  new_face = malloc(sizeof(Face));
  if (NULL == new_face) {
    return NULL;
  }
  new_face->loops = NULL;
  memset(new_face->normal, 0.0, 3);

  loop_list_prepend(&(new_face->loops), v1, e1);
  loop_list_prepend(&(new_face->loops), v2, e2);
  loop_list_prepend(&(new_face->loops), v3, e3);

  MeshFaceItem *new_face_item = list_prepend(&(mesh->face_items), new_face);

  list_prepend(&(VERT_FROM_VERT_ITEM(v1)->link_face_items), new_face_item);
  list_prepend(&(VERT_FROM_VERT_ITEM(v2)->link_face_items), new_face_item);
  list_prepend(&(VERT_FROM_VERT_ITEM(v3)->link_face_items), new_face_item);

  list_prepend(&(EDGE_FROM_EDGE_ITEM(e1)->link_face_items), new_face_item);
  list_prepend(&(EDGE_FROM_EDGE_ITEM(e1)->link_face_items), new_face_item);
  list_prepend(&(EDGE_FROM_EDGE_ITEM(e3)->link_face_items), new_face_item);

  if (already_exists) {
    *already_exists = false;
  }
  return new_face_item;
}

/* Does not remove face vertices or edges */
void face_remove_keep_edges_verts(MeshFaceItem *face_item) {
  list_free(&(FACE_FROM_FACE_ITEM(face_item)->loops), NULL);
  list_remove_item(face_item, NULL);
}

void edge_remove_keep_verts(MeshEdgeItem *edge_item) {
  list_free(&(EDGE_FROM_EDGE_ITEM(edge_item)->link_face_items),
            (ListDataFreeFuncPointer)&face_remove_keep_edges_verts);
  list_remove_item(edge_item, NULL);
}

/* This also removes faces and edges that contain the vertex */
void vert_remove(MeshVertItem *vert_item) {
  list_free(&(VERT_FROM_VERT_ITEM(vert_item)->link_edge_items),
            (ListDataFreeFuncPointer)&edge_remove_keep_verts);
  list_free(&(VERT_FROM_VERT_ITEM(vert_item)->link_face_items),
            (ListDataFreeFuncPointer)&face_remove_keep_edges_verts);
  list_remove_item(vert_item, NULL);
}

void mesh_free(Mesh **mesh_ref) {
  /* TODO: free all verts/edges/faces without vert_remove (without worrying
   * about freeing linked geo, as all geo would be freed anyways)*/
  // list_free(&((*mesh_ref)->vert_items),
  // (ListDataFreeFuncPointer)&vert_remove);
  free(*mesh_ref);
  *mesh_ref = NULL;
}