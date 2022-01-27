#ifndef MESH_STL
#define MESH_STL

typedef struct Triangle {
  float normal[3];
  union {
    struct {
      float v1[3];
      float v2[3];
      float v3[3];
    };
    float vertices[3][3];
  };
} Triangle;

typedef struct TriangleList {
  Triangle *data;
  struct TriangleList *next;
} TriangleList;

void free_triangles(TriangleList **head_ref);
TriangleList *read_stl(char *filepath);

#endif