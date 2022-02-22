#ifndef MESH_HPP
#define MESH_HPP

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>

#define POOLMEM_NODES_PER_BLOCK_DEFAULT 10240U

struct Vert;
struct Edge;
struct Face;

template <typename T> struct PoolMemNode {
  T data;
  struct PoolMemNode *next = nullptr;
};
template <typename T> struct PoolMem {
private:
  std::vector<PoolMemNode<T> *> blocks;
  PoolMemNode<T> *allocation_pointer = nullptr;

  inline PoolMemNode<T> *allocate_block() {
    auto block_begin = new PoolMemNode<T>[POOLMEM_NODES_PER_BLOCK_DEFAULT]();
    blocks.push_back(block_begin);
    auto it = block_begin;
    for (int i = 0; i < POOLMEM_NODES_PER_BLOCK_DEFAULT - 1; i++) {
      it->next = (it + 1);
      it = it->next;
    }
    it->next = nullptr;
    return block_begin;
  }

public:
  ~PoolMem() {
    for (auto mem : blocks) {
      delete[] mem;
    }
  }

  inline PoolMemNode<T> *allocate() {
    if (allocation_pointer == nullptr) {
      allocation_pointer = allocate_block();
    }

    auto free_node = allocation_pointer;
    allocation_pointer = allocation_pointer->next;
    return free_node;
  }

  inline void deallocate(PoolMemNode<T> *node) {
    node->next = allocation_pointer;
    allocation_pointer = node;
  }
};

template <typename T> static inline void vector_remove_by_value(std::vector<T> vec, T value) {
  auto it = std::find(vec.begin(), vec.end(), value);
  if (it != vec.end()) {
    vec.erase(it);
  }
}

struct Vert {
  PoolMemNode<Vert> *mem = nullptr;

  float location[3];
  std::vector<Edge *> link_edges;
  std::vector<Face *> link_faces;

  struct Vert *next, *prev;

  inline void remove_link_edge(Edge *edge) { vector_remove_by_value<Edge *>(link_edges, edge); }

  inline void remove_link_face(Face *face) { vector_remove_by_value<Face *>(link_faces, face); }
};

struct Edge {
  PoolMemNode<Edge> *mem = nullptr;

  Vert *v1, *v2;
  std::vector<Face *> link_faces;

  struct Edge *next, *prev;

  inline void remove_link_face(Face *face) { vector_remove_by_value<Face *>(link_faces, face); }
};

struct Face {
  PoolMemNode<Face> *mem = nullptr;

  float custom_normal[3];
  std::vector<Vert *> verts;
  std::vector<Edge *> edges;

  struct Face *next, *prev;
};

struct Mesh {
public:
  // List heads
  Vert *verts = nullptr;
  Edge *edges = nullptr;
  Face *faces = nullptr;

  PoolMem<Vert> verts_pool;
  PoolMem<Edge> edges_pool;
  PoolMem<Face> faces_pool;

  inline Vert *vert_create(float location[3]) {
    auto vert = verts_pool.allocate();
    vert->data.location[0] = location[0];
    vert->data.location[1] = location[1];
    vert->data.location[2] = location[2];
    return &(vert->data);
  }

  inline Vert *vert_create(float v1, float v2, float v3) {
    auto vert = verts_pool.allocate();
    vert->data.location[0] = v1;
    vert->data.location[1] = v2;
    vert->data.location[2] = v3;

    // Prepend vertex
    vert->data.next = verts;
    vert->data.prev = nullptr;
    if (verts != nullptr) {
      verts->prev = &(vert->data);
    }
    verts = &(vert->data);

    return &(vert->data);
  }

  inline void vert_remove(Vert *v) {
    auto link_edges_copy = v->link_edges;
    for (auto e : link_edges_copy) {
      edge_remove_keep_verts(e);
    }

    auto link_faces_copy = v->link_faces;
    for (auto f : link_faces_copy) {
      face_remove_keep_verts_edges(f);
    }

    // Remove vertex
    auto prev = v->prev;
    auto next = v->next;
    if (prev != nullptr) {
      prev->next = next;
    }
    if (next != nullptr) {
      next->prev = prev;
    }
    verts_pool.deallocate(v->mem);
  }

  inline Edge *edge_create(Vert *v1, Vert *v2) {
    auto edge_mem = edges_pool.allocate();
    auto edge = &(edge_mem->data);

    edge->v1 = v1;
    edge->v2 = v2;

    v1->link_edges.push_back(edge);
    v2->link_edges.push_back(edge);

    // Prepend edge
    edge->next = edges;
    edge->prev = nullptr;
    if (edges != nullptr) {
      edges->prev = edge;
    }
    edges = edge;

    return edge;
  }

  inline void edge_remove_keep_verts(Edge *e) {
    e->v1->remove_link_edge(e);
    e->v2->remove_link_edge(e);

    for (auto face : e->link_faces) {
      face_remove_keep_verts_edges(face);
    }

    // Remove edge
    auto prev = e->prev;
    auto next = e->next;
    if (prev != nullptr) {
      prev->next = next;
    }
    if (next != nullptr) {
      next->prev = prev;
    }

    edges_pool.deallocate(e->mem);
  }

  inline void edge_remove(Edge *e) {
    // Store pointers to verts, as vert_remove destroys linked edges, including this very edge
    auto v1 = e->v1;
    auto v2 = e->v2;
    vert_remove(v1);
    vert_remove(v2);
  }

  inline Face *face_create(Vert *vert_ids[3]) {
    auto e1 = edge_create(vert_ids[0], vert_ids[1]);
    auto e2 = edge_create(vert_ids[1], vert_ids[2]);
    auto e3 = edge_create(vert_ids[2], vert_ids[0]);

    auto face_mem = faces_pool.allocate();
    auto face = &(face_mem->data);

    // TODO: get rid of std::vector
    face->verts = {vert_ids[0], vert_ids[1], vert_ids[2]};
    face->edges = {e1, e2, e3};

    e1->link_faces.push_back(face);
    e2->link_faces.push_back(face);
    e3->link_faces.push_back(face);

    vert_ids[0]->link_faces.push_back(face);
    vert_ids[1]->link_faces.push_back(face);
    vert_ids[2]->link_faces.push_back(face);

    // Prepend face
    face->next = faces;
    face->prev = nullptr;
    if (faces != nullptr) {
      faces->prev = face;
    }
    faces = face;

    return face;
  }

  inline void face_remove_keep_verts_edges(Face *f) {
    auto edges_copy = f->edges;
    auto verts_copy = f->verts;

    for (auto e : edges_copy) {
      e->remove_link_face(f);
    }
    for (auto v : verts_copy) {
      v->remove_link_face(f);
    }

    // Remove edge
    auto prev = f->prev;
    auto next = f->next;
    if (prev != nullptr) {
      prev->next = next;
    }
    if (next != nullptr) {
      next->prev = prev;
    }

    faces_pool.deallocate(f->mem);
  }
};

#endif
