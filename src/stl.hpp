#ifndef STL_HPP
#define STL_HPP

#include <vector>

#include "vec3.hh"

class Mesh
{
private:
  size_t capacity = 0;

public:
  Vec3 *verts = nullptr;
  size_t count = 0;
  Mesh(size_t num_verts_initial = 0)
  {
    this->capacity = (num_verts_initial > 0) ? num_verts_initial : 1;
    this->verts = new Vec3[capacity];
  }
  void reserve(size_t new_capacity)
  {
    if (new_capacity > this->capacity)
    {
      this->capacity = new_capacity;
      this->verts = (Vec3 *)realloc(this->verts, this->capacity * sizeof(Vec3));
    }
  }
  void add_vertex(float x, float y, float z)
  {
    if (this->count >= this->capacity)
    {
      this->reserve(this->capacity * 2);
    }
    this->verts[count].x = x;
    this->verts[count].y = y;
    this->verts[count].z = z;
    this->count++;
  }
  void free()
  {
    if (this->verts)
    {
      delete[] this->verts;
    }
  }
};

void read_stl(Mesh &mesh, const char *filepath);

#endif