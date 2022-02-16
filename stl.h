#ifndef STL_H
#define STL_H

#include "buffer.h"

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

Buffer *read_stl(char *filepath);

#endif