#ifndef MESH_HPP
#define MESH_HPP

#include <list>

struct Triangle {
  float custom_normal[3];
  union {
    float vertices[3][3];
    struct {
      float v1[3], v2[3], v3[3];
    };
  };
};

struct Mesh {
  std::list<Triangle> triangles;
};

#endif
