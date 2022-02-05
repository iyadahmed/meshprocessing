#ifndef MESH_HPP
#define MESH_HPP

#include <list>

struct EdgeData;
struct VertData;
struct FaceData;

struct VertData {
  float location[3];
  std::list<std::list<EdgeData>::iterator> link_edges;
  std::list<std::list<FaceData>::iterator> link_faces;
};

struct EdgeData {
  std::list<VertData>::iterator v1;
  std::list<VertData>::iterator v2;
  std::list<std::list<FaceData>::iterator> link_faces;
};

struct Loop {
  std::list<VertData>::iterator vert;
  std::list<EdgeData>::iterator edge;
};

struct FaceData {
  float normal[3];
  std::list<Loop> loops;
};

struct Mesh {
  std::list<VertData> verts;
  std::list<EdgeData> edges;
  std::list<FaceData> faces;
};

std::list<VertData>::iterator create_vertex(Mesh *mesh, float location[3]);
std::list<EdgeData>::iterator create_edge(Mesh *mesh,
                                          std::list<VertData>::iterator v1,
                                          std::list<VertData>::iterator v2,
                                          bool *already_exists);
std::list<FaceData>::iterator create_face(Mesh *mesh,
                                          std::list<VertData>::iterator v1,
                                          std::list<VertData>::iterator v2,
                                          std::list<VertData>::iterator v3,
                                          bool *already_exists);

#endif