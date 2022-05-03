#pragma once

#include "buffer.hh"
#include "../mesh.hh"

/*  ASCII STL spec.:
 *  solid name
 *    facet normal ni nj nk
 *      outer loop
 *        vertex v1x v1y v1z
 *        vertex v2x v2y v2z
 *        vertex v3x v3y v3z
 *      endloop
 *    endfacet
 *    ...
 *  endsolid name
 */

static inline void read_stl_ascii_vertex(Mesh &mesh, Buffer &buf)
{
    float float3_buf[3];
    for (int i = 0; i < 3; i++)
    {
        if (!buf.parse_float(float3_buf + i))
        {
            puts("STL Importer: ERROR! failed to parse float.");
            return;
        }
    }
    mesh.add_vertex(float3_buf[0], float3_buf[1], float3_buf[2]);
}

static void read_stl_ascii(Mesh &mesh, FILE *file)
{
    char tmp[1024];
    fseek(file, 0, SEEK_SET);
    fgets(tmp, 1024, file); // Skip header line

    auto buf = Buffer::from_file(file);
    while (!buf.is_end())
    {
        if (buf.parse_token("vertex", 6))
        {
            read_stl_ascii_vertex(mesh, buf);
        }
        else
        {
            buf.drop_leading_whitespace();
            buf.drop_leading_non_whitespace();
        }
    }
    buf.free();
}