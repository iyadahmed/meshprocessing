#pragma once

#include <cctype>
#include <cerrno>
#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdio.h>

#include "../mesh.hh"

const size_t BINARY_HEADER = 80;
const size_t BINARY_STRIDE = 12 * 4 + 2;

#pragma pack(push, 1)
struct STLBinaryTriangle
{
    float normal[3];
    union
    {
        struct
        {
            float v1[3], v2[3], v3[3];
        };
        float verts[3][3];
    };
    uint16_t attribute_byte_count;
};
#pragma pack(pop)

static inline void print_float(float value, int digit_count)
{
    char buf[128];
    gcvt(value, digit_count, buf);
    puts(buf);
}

/*  Binary STL spec.:
 *   UINT8[80]    – Header                  - 80 bytes
 *   UINT32       – Number of triangles     - 4 bytes
 *   For each triangle                      - 50 bytes:
 *     REAL32[3]   – Normal vector          - 12 bytes
 *     REAL32[3]   – Vertex 1               - 12 bytes
 *     REAL32[3]   – Vertex 2               - 12 bytes
 *     REAL32[3]   – Vertex 3               - 12 bytes
 *     UINT16      – Attribute byte count   -  2 bytes
 */
static STLBinaryTriangle *read_stl_binary_core(FILE *file,
                                               size_t *num_read_tris)
{
    uint32_t num_tri = 0;
    fseek(file, BINARY_HEADER, SEEK_SET);
    if (fread(&num_tri, sizeof(uint32_t), 1, file) < 1)
    {
        puts("STL Importer: Failed to read binary STL triangle count.");
        return NULL;
    }
    auto tris = new STLBinaryTriangle[num_tri];
    *num_read_tris = fread(tris, sizeof(STLBinaryTriangle), num_tri, file);
    return tris;
}

static void read_stl_binary(TriMesh &mesh, FILE *file)
{
    size_t num_tris;
    Triangle current_triangle{};
    STLBinaryTriangle *tris = read_stl_binary_core(file, &num_tris);
    if (tris == NULL)
    {
        return;
    }
    mesh.reserve(num_tris * 3);
    for (size_t i = 0; i < num_tris; i++)
    {
        for (short j = 0; j < 3; j++)
        {
            memcpy(current_triangle.verts, tris[i].verts, sizeof(float[3][3]));
        }
        mesh.add_triangle(current_triangle);
    }
    delete[] tris;
}
