#include <stdexcept>

#include "trimesh.hh"

TriMesh::TriMesh(size_t tris_num)
{
    cap_ = (tris_num > 0) ? tris_num : 1;
    tris_ = new Triangle[cap_];
}

size_t TriMesh::tris_count() const
{
    return tris_num_;
}

Triangle &TriMesh::get_tri(size_t index)
{
    if (index < tris_num_)
    {
        return tris_[index];
    }
    throw std::out_of_range("Index out of range");
}

void TriMesh::reserve(size_t tris_num)
{
    if (tris_num > cap_)
    {
        cap_ = tris_num;
        tris_ = (Triangle *)realloc(tris_, cap_ * sizeof(Triangle));
    }
}

void TriMesh::add_triangle(const Triangle &triangle)
{
    if (tris_num_ >= cap_)
    {
        reserve(cap_ * 2);
    }
    tris_[tris_num_] = triangle;
    tris_num_++;
}

void TriMesh::free()
{
    if (tris_)
    {
        delete[] tris_;
    }
}