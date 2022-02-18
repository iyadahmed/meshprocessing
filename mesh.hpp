#include <unordered_map>
#include <vector>

struct Vert {
  float location[3];
  std::vector<uint32_t> link_edges_ids;
  std::vector<uint32_t> link_faces_ids;
};

struct Edge {
  uint32_t v1_id, v2_id;
  std::vector<uint32_t> link_faces_ids;
};

struct Face {
  std::vector<uint32_t> verts_ids;
  std::vector<uint32_t> edges_ids;
};

// TODO: reduce memory allocations when creating vertices (insertind into unordered dict)
struct Mesh {
private:
  uint32_t vert_id_counter;
  uint32_t edge_id_counter;
  uint32_t face_id_counter;

public:
  std::unordered_map<uint32_t, Vert> verts;
  std::unordered_map<uint32_t, Edge> edges;
  std::unordered_map<uint32_t, Face> faces;

  inline uint32_t vert_create(float x, float y, float z) {
    auto vert_id = vert_id_counter;
    verts[vert_id] = {{x, y, z}, {}, {}};
    vert_id_counter++;
    return vert_id;
  }

  inline void vert_remove(uint32_t vert_id) {
    auto v = verts[vert_id];
    for (auto edge_id : v.link_edges_ids) {
      edge_remove_keep_verts(edge_id);
    }
    for (auto face_id : v.link_edges_ids) {
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

  inline void edge_remove_keep_verts(uint32_t edge_id) {}

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
        {vert_ids[0], vert_ids[0], vert_ids[0]},
        {e1_id, e2_id, e3_id},
    };
    face_id_counter++;
    return face_id;
  }

  inline void face_remove_keep_verts_edges(uint32_t face_id) {
    // TODO
  }

  inline void face_remove(uint32_t face_id) {
    // TODO
  }
};
