#pragma once

#include "vec3.hh"

union Triangle
{

    struct
    {
        float v1[3], v2[3], v3[3];
    };
    float verts[3][3];
};

class TriMesh
{
private:
    size_t m_capacity = 0;
    Triangle *m_tris = nullptr;
    size_t m_count = 0;

public:
    TriMesh(size_t num_verts_initial = 0)
    {
        m_capacity = (num_verts_initial > 0) ? num_verts_initial : 1;
        m_tris = new Triangle[m_capacity];
    }
    /* Number of Triangles in mesh */
    size_t count() const
    {
        return m_count;
    }
    Triangle &get_tri(size_t index)
    {
        if (index < m_count)
        {
            return m_tris[index];
        }
        throw std::out_of_range("Index out of range");
    }
    void reserve(size_t new_capacity)
    {
        if (new_capacity > m_capacity)
        {
            m_capacity = new_capacity;
            m_tris = (Triangle *)realloc(m_tris, m_capacity * sizeof(Triangle));
        }
    }
    void add_triangle(Triangle &triangle)
    {
        if (m_count >= m_capacity)
        {
            reserve(m_capacity * 2);
        }
        m_tris[m_count] = triangle;
        m_count++;
    }
    void free()
    {
        if (m_tris)
        {
            delete[] m_tris;
        }
    }
};