#pragma once

#include "../mesh.hh"
#include "buffer.hh"

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

static inline void read_stl_ascii_vertex(float out[3], Buffer &buf)
{
    for (int i = 0; i < 3; i++)
    {
        if (!buf.parse_float(out + i))
        {
            puts("STL Importer: ERROR! failed to parse float.");
            return;
        }
    }
}

static void read_stl_ascii(TriMesh &mesh, FILE *file)
{
    char tmp[1024];
    fseek(file, 0, SEEK_SET);
    fgets(tmp, 1024, file); // Skip header line

    Triangle current_triangle{};

    auto buf = Buffer::from_file(file);

    while (!buf.is_end())
    {
        if (buf.parse_token("vertex", 6))
        {
            read_stl_ascii_vertex(current_triangle.v1, buf);
            if (buf.parse_token("vertex", 6))
            {
                read_stl_ascii_vertex(current_triangle.v2, buf);
            }
            else
            {
                fprintf(stderr, "Token mismatch, expected \"vertex\", found ");
                buf.fprintn(stderr, 6);
                fputc('\n', stderr);
            }
            if (buf.parse_token("vertex", 6))
            {
                read_stl_ascii_vertex(current_triangle.v3, buf);
            }
            else
            {
                fprintf(stderr, "Token mismatch, expected \"vertex\", found ");
                buf.fprintn(stderr, 6);
                fputc('\n', stderr);
            }
            mesh.add_triangle(current_triangle);
        }
        else
        {
            buf.drop_leading_whitespace();
            buf.drop_leading_non_whitespace();
        }
    }
    buf.free();
}