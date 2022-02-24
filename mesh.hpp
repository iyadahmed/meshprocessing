#ifndef MESH_HPP
#define MESH_HPP

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "list.hpp"
#include "poolmem.hpp"

struct Vert;
struct Edge;
struct Face;

struct Vert {
  PoolMemNode<Vert> *mem = nullptr;

  float location[3];

  List<ListNode<Edge *> *> link_edges;
  List<ListNode<Face *> *> link_faces;
};

struct Edge {
  PoolMemNode<Edge> *mem = nullptr;

  ListNode<Vert *> *v1, *v2;
  List<ListNode<Face *> *> link_faces;
};

struct Face {
  PoolMemNode<Face> *mem = nullptr;

  float custom_normal[3];
  List<ListNode<Vert *> *> verts;
  List<ListNode<Edge *> *> edges;
};

struct Mesh {
public:
  List<Vert *> verts;
  List<Edge *> edges;
  List<Face *> faces;

  PoolMem<Vert> verts_pool;
  PoolMem<Edge> edges_pool;
  PoolMem<Face> faces_pool;

  inline ListNode<Vert *> *vert_create(float location[3]) {
    auto vert_mem = verts_pool.allocate();
    vert_mem->data.location[0] = location[0];
    vert_mem->data.location[1] = location[1];
    vert_mem->data.location[2] = location[2];
    vert_mem->data.mem = vert_mem;
    return verts.prepend(&(vert_mem->data));
  }

  inline ListNode<Vert *> *vert_create(float v1, float v2, float v3) {
    auto vert_mem = verts_pool.allocate();
    vert_mem->data.location[0] = v1;
    vert_mem->data.location[1] = v2;
    vert_mem->data.location[2] = v3;
    vert_mem->data.mem = vert_mem;
    return verts.prepend(&(vert_mem->data));
  }

  inline void vert_remove(ListNode<Vert *> *v_list_node) {
    auto v = v_list_node->data;
    auto link_edges_copy = v->link_edges;

    auto link_edges_it = link_edges_copy.head;
    while (link_edges_it) {
      edge_remove_keep_verts(link_edges_it->data);
      link_edges_it = link_edges_it->next;
    }

    auto link_faces_copy = v->link_faces;
    auto link_faces_it = link_faces_copy.head;
    while (link_faces_it) {
      face_remove_keep_verts_edges(link_faces_it->data);
      link_faces_it = link_faces_it->next;
    }

    verts.remove(v_list_node);
    verts_pool.deallocate(v->mem);
  }

  inline ListNode<Edge *> *edge_create(ListNode<Vert *> *v1_list_node, ListNode<Vert *> *v2_list_node) {
    auto v1 = v1_list_node->data;
    auto v2 = v2_list_node->data;

    auto edge_mem = edges_pool.allocate();
    auto edge = &(edge_mem->data);

    edge->v1 = v1_list_node;
    edge->v2 = v2_list_node;

    auto e_list_node = edges.prepend(edge);

    v1->link_edges.prepend(e_list_node);
    v2->link_edges.prepend(e_list_node);

    return e_list_node;
  }

  inline void edge_remove_keep_verts(ListNode<Edge *> *e_list_node) {
    auto e = e_list_node->data;

    e->v1->data->link_edges.remove_by_value(e_list_node);
    e->v2->data->link_edges.remove_by_value(e_list_node);

    auto link_faces_it = e->link_faces.head;
    while (link_faces_it) {
      face_remove_keep_verts_edges(link_faces_it->data);
      link_faces_it = link_faces_it->next;
    }

    edges.remove(e_list_node);
    edges_pool.deallocate(e->mem);
  }

  inline void edge_remove(ListNode<Edge *> *e_list_node) {
    auto e = e_list_node->data;
    // Store pointers to verts, as vert_remove destroys linked edges, including this very edge
    auto v1 = e->v1;
    auto v2 = e->v2;
    vert_remove(v1);
    vert_remove(v2);
  }

  inline ListNode<Face *> *face_create(ListNode<Vert *> *vert_list_nodes[3]) {
    auto e1 = edge_create(vert_list_nodes[0], vert_list_nodes[1]);
    auto e2 = edge_create(vert_list_nodes[1], vert_list_nodes[2]);
    auto e3 = edge_create(vert_list_nodes[2], vert_list_nodes[0]);

    auto face_mem = faces_pool.allocate();
    auto face = &(face_mem->data);

    for (int i = 0; i < 3; i++) {
      face->verts.prepend(vert_list_nodes[i]);
    }

    face->edges.prepend(e1);
    face->edges.prepend(e2);
    face->edges.prepend(e3);

    auto face_list_node = faces.prepend(face);

    e1->data->link_faces.prepend(face_list_node);
    e2->data->link_faces.prepend(face_list_node);
    e3->data->link_faces.prepend(face_list_node);

    for (int i = 0; i < 3; i++) {
      vert_list_nodes[i]->data->link_faces.prepend(face_list_node);
    }

    return face_list_node;
  }

  inline void face_remove_keep_verts_edges(ListNode<Face *> *f_list_node) {
    auto f = f_list_node->data;

    auto edges_copy = f->edges;
    auto verts_copy = f->verts;

    auto edges_it = edges_copy.head;
    while (edges_it) {
      edges_it->data->data->link_faces.remove_by_value(f_list_node);
      edges_it = edges_it->next;
    }

    auto verts_it = verts_copy.head;
    while (verts_it) {
      verts_it->data->data->link_faces.remove_by_value(f_list_node);
      verts_it = verts_it->next;
    }

    faces.remove(f_list_node);
    faces_pool.deallocate(f->mem);
  }
};

#endif
