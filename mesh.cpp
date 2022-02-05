#include <iostream>

#include "mesh.hpp"

std::list<VertData>::iterator create_vertex(Mesh *mesh, float location[3]) {

  VertData new_vert_data;
  new_vert_data.location[0] = location[0];
  new_vert_data.location[1] = location[1];
  new_vert_data.location[2] = location[2];
  mesh->verts.push_front(new_vert_data);

  return mesh->verts.begin();
}

std::list<EdgeData>::iterator create_edge(Mesh *mesh,
                                          std::list<VertData>::iterator v1,
                                          std::list<VertData>::iterator v2,
                                          bool *already_exists) {
  EdgeData ed;
  for (auto e : (*v1).link_edges) {
    ed = *e;
    if ((ed.v1 == v1 && ed.v2 == v2) || (ed.v1 == v2 && ed.v2 == v1)) {
      if (already_exists) {
        *already_exists = true;
      }
      return e;
    }
  }
  EdgeData new_edge_data;
  new_edge_data.v1 = v1;
  new_edge_data.v2 = v2;
  if (already_exists) {
    *already_exists = false;
  }
  mesh->edges.push_front(new_edge_data);
  auto e = mesh->edges.begin();
  (*v1).link_edges.push_front(e);
  (*v2).link_edges.push_front(e);
  return e;
}

/* TODO: function for creating n-gons */
std::list<FaceData>::iterator create_face(Mesh *mesh,
                                          std::list<VertData>::iterator v1,
                                          std::list<VertData>::iterator v2,
                                          std::list<VertData>::iterator v3,
                                          bool *already_exists) {
  bool edge_already_exists[3] = {false};

  auto e1 = create_edge(mesh, v1, v2, edge_already_exists);
  auto e2 = create_edge(mesh, v2, v3, edge_already_exists + 1);
  auto e3 = create_edge(mesh, v3, v1, edge_already_exists + 2);

  /* Check if face already exists, return it if so */
  std::size_t num_edges_link_face = 0;
  bool edge_found[3] = {false};

  if (edge_already_exists[0] && edge_already_exists[1] &&
      edge_already_exists[2]) {
    for (auto link_face : (*v1).link_faces) {
      num_edges_link_face = 0;
      edge_found[0] = false;
      edge_found[1] = false;
      edge_found[2] = false;
      for (auto loop : (*link_face).loops) {
        if (loop.edge == e1) {
          edge_found[0] = true;
        } else if (loop.edge == e2) {
          edge_found[1] = true;
        } else if (loop.edge == e3) {
          edge_found[2] = true;
        }
        num_edges_link_face++;
      }
      if (edge_found[0] && edge_found[1] && edge_found[2] &&
          (num_edges_link_face == 3)) {
        if (already_exists) {
          *already_exists = true;
        }
        return link_face;
      }
    }
  }

  FaceData new_face_data;
  new_face_data.loops.push_front({v1, e1});
  new_face_data.loops.push_front({v2, e2});
  new_face_data.loops.push_front({v3, e3});

  mesh->faces.push_front(new_face_data);
  auto new_face = mesh->faces.begin();
  (*v1).link_faces.push_front(new_face);
  (*v2).link_faces.push_front(new_face);
  (*v3).link_faces.push_front(new_face);

  (*e1).link_faces.push_front(new_face);
  (*e2).link_faces.push_front(new_face);
  (*e3).link_faces.push_front(new_face);

  if (already_exists) {
    *already_exists = false;
  }

  return new_face;
}