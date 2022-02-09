#ifndef MESH_H
#define MESH_H

typedef struct Triangle {
  float normal[3]; /* Custom normal */
  union {
    struct {
      float v1[3];
      float v2[3];
      float v3[3];
    };
    float vertices[3][3];
  };

  struct Triangle *next;
} Triangle;

void prepend_triangle(Triangle **head_ref, const Triangle *tri);
void free_triangles(Triangle **head_ref);

#endif