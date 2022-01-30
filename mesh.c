#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "mesh.h"

Vert *create_vertex(Mesh *mesh, float location[3]) {
  Vert *new_vert = malloc(sizeof(Vert));
  if (NULL == new_vert) {
    return NULL;
  }
  new_vert->link_edges = NULL;
  new_vert->link_faces = NULL;
  memcpy(new_vert->location, location, sizeof(float[3]));
  prepend(&(mesh->vertices), new_vert);
  return new_vert;
}

Edge *create_edge(Mesh *mesh, Vert *v1, Vert *v2, bool *already_exists) {
  EdgeList *link_edge_iter = v1->link_edges;
  Edge *link_edge = NULL;
  while (link_edge_iter) {
    link_edge = link_edge_iter->data;
    if (((link_edge->v1 == v1) && (link_edge->v2 == v2)) ||
        ((link_edge->v1 == v2) && (link_edge->v2 == v1))) {
      if (already_exists) {
        *already_exists = true;
      }
      return link_edge;
    }
    link_edge_iter = link_edge_iter->next;
  }

  Edge *new_edge = malloc(sizeof(Edge));
  if (NULL == new_edge) {
    return NULL;
  }
  new_edge->link_faces = NULL;
  new_edge->v1 = v1;
  new_edge->v2 = v2;

  prepend(&(mesh->edges), new_edge);
  prepend(&(v1->link_edges), new_edge);
  prepend(&(v2->link_edges), new_edge);

  if (already_exists) {
    *already_exists = false;
  }
  return new_edge;
}

static void loop_prepend(Loop **loop_first_ref, Vert *vert, Edge *edge) {
  Loop *new_loop = malloc(sizeof(Loop));
  if (NULL == new_loop) {
    return;
  }
  new_loop->vert = vert;
  new_loop->edge = edge;
  new_loop->next = *loop_first_ref;
  *loop_first_ref = new_loop;
}

/* TODO: function for creating n-gons */
Face *create_face(Mesh *mesh, Vert *v1, Vert *v2, Vert *v3,
                  bool *already_exists) {
  Face *new_face = NULL;
  Edge *e1 = NULL, *e2 = NULL, *e3 = NULL;
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
  FaceList *link_faces_iter = v1->link_faces;
  Face *link_face = NULL;
  Loop *link_face_loops_iter = NULL;
  Edge *link_face_edge = NULL;
  size_t num_edges_link_face = 0;
  bool edge_found[3] = {false};

  if (edge_already_exists[0] && edge_already_exists[1] &&
      edge_already_exists[2]) {
    while (link_faces_iter) {
      link_face = link_faces_iter->data;
      link_face_loops_iter = link_face->loop_first;

      num_edges_link_face = 0;
      memset(edge_found, false, 3);
      while (link_face_loops_iter) {
        link_face_edge = link_face_loops_iter->edge;

        if (link_face_edge == e1) {
          edge_found[0] = true;
        } else if (link_face_edge == e2) {
          edge_found[1] = true;
        } else if (link_face_edge == e3) {
          edge_found[2] = true;
        }
        link_face_loops_iter = link_face_loops_iter->next;
        num_edges_link_face++;
      }

      if (edge_found[0] && edge_found[1] && edge_found[2] &&
          (num_edges_link_face == 3)) {
        if (already_exists) {
          *already_exists = true;
          return link_face;
        }
      }

      link_faces_iter = link_faces_iter->next;
    }
  }

  new_face = malloc(sizeof(Face));
  if (NULL == new_face) {
    return NULL;
  }
  new_face->loop_first = NULL;
  memset(new_face->normal, 0.0, 3);

  loop_prepend(&(new_face->loop_first), v1, e1);
  loop_prepend(&(new_face->loop_first), v2, e2);
  loop_prepend(&(new_face->loop_first), v3, e3);

  prepend(&(mesh->faces), new_face);

  prepend(&(v1->link_faces), new_face);
  prepend(&(v2->link_faces), new_face);
  prepend(&(v3->link_faces), new_face);

  prepend(&(e1->link_faces), new_face);
  prepend(&(e2->link_faces), new_face);
  prepend(&(e3->link_faces), new_face);

  if (already_exists) {
    *already_exists = false;
  }
  return new_face;
}