#include <algorithm>
#include <vector>

#include "buffer.hpp"

struct Vert;
struct Edge;
struct Face;

struct Vert {
  float location[3];
  std::vector<Edge *> link_edges;
  std::vector<Face *> link_faces;

  bool operator==(const Vert &other) {
    // Compare by address since vertices are supposed to belong to a mesh
    return this == &other;
  }
};

struct Edge {
  Vert *v1, *v2;
  std::vector<Face *> link_faces;

  bool operator==(const Edge &other) {
    // Compare by address since edges are supposed to belong to a mesh
    return this == &other;
  }
};

struct Face {
  std::vector<Vert *> verts;
  std::vector<Edge *> edges;

  bool operator==(const Face &other) {
    // Compare by address since faces are supposed to belong to a mesh
    return this == &other;
  }
};

struct Mesh {
  std::vector<Vert> verts;
  std::vector<Edge> edges;
  std::vector<Face> faces;

  inline Vert *vert_create(float x, float y, float z) {
    Vert v;
    v.location[0] = x;
    v.location[1] = y;
    v.location[2] = z;
    verts.push_back(v);
    return &verts.back();
  }

  inline void vert_remove(Vert *v) {
    // TODO
  }

  inline Edge *edge_create(Vert *v1, Vert *v2) {
    Edge e;
    e.v1 = v1;
    e.v2 = v2;
    edges.push_back(e);
    return &edges.back();
  }

  inline void edge_remove_keep_verts_faces(Edge *e) {
    // TODO
  }

  inline void edge_remove(Edge *e) {
    Vert *v1 = e->v1;
    Vert *v2 = e->v2;
    vert_remove(v1);
    vert_remove(v2);
  }

  inline Face *face_create(Vert *v1, Vert *v2, Vert *v3) {
    Face f;
    f.verts.push_back(v1);
    f.verts.push_back(v2);
    f.verts.push_back(v3);
    auto e1 = edge_create(v1, v2);
    auto e2 = edge_create(v2, v3);
    auto e3 = edge_create(v3, v1);
    f.edges.push_back(e1);
    f.edges.push_back(e2);
    f.edges.push_back(e3);
    faces.push_back(f);
    return &faces.back();
  }

  inline void face_remove_keep_verts_edges(Face *f) {
    // for (auto v : f->verts) {
    //   v->link_faces.erase(std::find(v->link_faces.begin(), v->link_faces.end(), f));
    // }
    // for (auto e : f->edges) {
    //   e->link_faces.erase(std::find(e->link_faces.begin(), e->link_faces.end(), f));
    // }
    // faces.erase(std::find(faces.begin(), faces.end(), *f));
  }

  inline void face_remove(Face *f) {
  }
};