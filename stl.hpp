#ifndef STL_HPP
#define STL_HPP

#include "buffer.hpp"

typedef struct STLTriangle {
  float normal[3]; /* Custom normal */
  union {
    struct {
      float v1[3];
      float v2[3];
      float v3[3];
    };
    float vertices[3][3];
  };
} STLTriangle;

Buffer<STLTriangle> *read_stl(char *filepath);

#endif