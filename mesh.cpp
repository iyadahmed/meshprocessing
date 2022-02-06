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
                                          std::list<VertData>::iterator v2) {
  EdgeData new_edge_data;
  new_edge_data.v1 = v1;
  new_edge_data.v2 = v2;
  mesh->edges.push_front(new_edge_data);
  auto e = mesh->edges.begin();
  v1->link_edges.push_front(e);
  v2->link_edges.push_front(e);
  return e;
}

/* TODO: function for creating n-gons */
std::list<FaceData>::iterator create_face(Mesh *mesh,
                                          std::list<VertData>::iterator v1,
                                          std::list<VertData>::iterator v2,
                                          std::list<VertData>::iterator v3) {

  auto e1 = create_edge(mesh, v1, v2);
  auto e2 = create_edge(mesh, v2, v3);
  auto e3 = create_edge(mesh, v3, v1);

  FaceData new_face_data;
  new_face_data.loops.push_front({v1, e1});
  new_face_data.loops.push_front({v2, e2});
  new_face_data.loops.push_front({v3, e3});

  mesh->faces.push_front(new_face_data);
  auto new_face = mesh->faces.begin();
  v1->link_faces.push_front(new_face);
  v2->link_faces.push_front(new_face);
  v3->link_faces.push_front(new_face);

  e1->link_faces.push_front(new_face);
  e2->link_faces.push_front(new_face);
  e3->link_faces.push_front(new_face);

  return new_face;
}

void face_remove(Mesh *mesh, std::list<FaceData>::iterator &face) {
  for (auto loop : face->loops) {
    loop.edge->link_faces.remove(face);
    loop.vert->link_faces.remove(face);
  }
  mesh->faces.erase(face);
  face = mesh->faces.end();
}