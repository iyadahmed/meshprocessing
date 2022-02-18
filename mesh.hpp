#ifndef MESH_HPP
#define MESH_HPP

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>

struct Vert {
  float location[3];
  std::vector<uint32_t> link_edges_ids;
  std::vector<uint32_t> link_faces_ids;

  inline void remove_link_edge(uint32_t edge_id) {
    auto it = std::find(link_edges_ids.begin(), link_edges_ids.end(), edge_id);
    if (it != link_edges_ids.end()) {
      link_edges_ids.erase(it);
    }
  }
  inline void remove_link_face(uint32_t face_id) {
    auto it = std::find(link_faces_ids.begin(), link_faces_ids.end(), face_id);
    if (it != link_faces_ids.end()) {
      link_faces_ids.erase(it);
    }
  }
};

struct Edge {
  uint32_t v1_id, v2_id;
  std::vector<uint32_t> link_faces_ids;

  inline void remove_link_face(uint32_t face_id) {
    auto it = std::find(link_faces_ids.begin(), link_faces_ids.end(), face_id);
    if (it != link_faces_ids.end()) {
      link_faces_ids.erase(it);
    }
  }
};

struct Face {
  float custom_normal[3];
  std::vector<uint32_t> verts_ids;
  std::vector<uint32_t> edges_ids;
};

// TODO: reduce memory allocations when creating vertices (inserting into unordered dict)
struct Mesh {
private:
  uint32_t vert_id_counter;
  uint32_t edge_id_counter;
  uint32_t face_id_counter;

public:
  std::unordered_map<uint32_t, Vert> verts;
  std::unordered_map<uint32_t, Edge> edges;
  std::unordered_map<uint32_t, Face> faces;

  inline uint32_t vert_create(float location[3]) {
    auto vert_id = vert_id_counter;
    verts[vert_id] = {{location[0], location[1], location[2]}, {}, {}};
    vert_id_counter++;
    return vert_id;
  }

  inline uint32_t vert_create(float v1, float v2, float v3) {
    auto vert_id = vert_id_counter;
    verts[vert_id] = {{v1, v2, v3}, {}, {}};
    vert_id_counter++;
    return vert_id;
  }

  inline void vert_remove(uint32_t vert_id) {
    auto v = verts[vert_id];
    auto link_edges_ids_copy = v.link_edges_ids;
    for (auto edge_id : link_edges_ids_copy) {
      edge_remove_keep_verts(edge_id);
    }

    auto link_faces_ids_copy = v.link_faces_ids;
    for (auto face_id : link_faces_ids_copy) {
      face_remove_keep_verts_edges(face_id);
    }
    verts.erase(vert_id);
  }

  inline uint32_t edge_create(uint32_t v1_id, uint32_t v2_id) {
    auto edge_id = edge_id_counter;
    edges[edge_id] = {v1_id, v2_id, {}};
    edge_id_counter++;
    return edge_id;
  }

  inline void edge_remove_keep_verts(uint32_t edge_id) {
    auto e = edges[edge_id];
    verts[e.v1_id].remove_link_edge(edge_id);
    verts[e.v2_id].remove_link_edge(edge_id);

    for (auto face_id : e.link_faces_ids) {
      face_remove_keep_verts_edges(face_id);
    }
    edges.erase(edge_id);
  }

  inline void edge_remove(uint32_t edge_id) {
    // Store pointer to verts, as vert_remove destroys linked edges, including this very edge
    auto e = edges[edge_id];
    auto v1_id = e.v1_id;
    auto v2_id = e.v2_id;
    vert_remove(v1_id);
    vert_remove(v2_id);
  }

  inline uint32_t face_create(uint32_t vert_ids[3]) {
    auto e1_id = edge_create(vert_ids[0], vert_ids[1]);
    auto e2_id = edge_create(vert_ids[1], vert_ids[2]);
    auto e3_id = edge_create(vert_ids[2], vert_ids[0]);
    auto face_id = face_id_counter;
    faces[face_id] = {
        {},
        {vert_ids[0], vert_ids[0], vert_ids[0]},
        {e1_id, e2_id, e3_id},
    };
    face_id_counter++;
    return face_id;
  }

  inline void face_remove_keep_verts_edges(uint32_t face_id) {
    auto f = faces[face_id];
    for (auto edge_id : f.edges_ids) {
      edges[edge_id].remove_link_face(face_id);
    }
    for (auto vert_id : f.verts_ids) {
      verts[vert_id].remove_link_face(face_id);
    }
    faces.erase(face_id);
  }
};

#endif
