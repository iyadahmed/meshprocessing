#pragma once

#include <cstddef>

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
    Triangle *tris_ = nullptr;
    size_t cap_ = 0;
    size_t tris_num_ = 0;

public:
    TriMesh(size_t tris_num = 0);
    size_t tris_count() const;

    Triangle &get_tri(size_t index);
    void add_triangle(const Triangle &triangle);

    void reserve(size_t tris_num);
    void free();
};