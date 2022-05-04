#include <cstdio>
#include <cstdint>

#include "importer.hh"
#include "ascii.hh"
#include "binary.hh"

/* Based on Blender's Python STL importer
 * https://github.com/blender/blender-addons/blob/599a8db33c45c2ad94f8d482f01b281252799770/io_mesh_stl/stl_utils.py#L62
 * Could check for "solid", but some files don't adhere */
static bool is_ascii_stl(FILE *file)
{
    fseek(file, BINARY_HEADER, SEEK_SET);
    uint32_t num_tri = 0;
    fread(&num_tri, sizeof(uint32_t), 1, file);
    if (num_tri == 0)
    {
        /* Number of triangles is 0, assume invalid binary */
        fputs("STL Importer: WARNING! Reported size (facet number) is 0, assuming "
              "invalid binary STL file.",
              stderr);
        return false;
    }
    long file_size = calc_file_size(file);
    return (file_size != BINARY_HEADER + 4 + BINARY_STRIDE * num_tri);
}

void read_stl(TriMesh &mesh, const char *filepath)
{
    /* TODO: support utf8 paths on Windows */
    FILE *file = fopen(filepath, "rb");

    if (file == NULL)
    {
        fprintf(stderr, "Error opening file: %s\n", filepath);
        return;
    }

    if (is_ascii_stl(file))
    {
        printf("Reading ASCII STL: %s\n", filepath);
        read_stl_ascii(mesh, file);
    }
    else
    {
        printf("Reading BINARY STL: %s\n", filepath);
        read_stl_binary(mesh, file);
    }
    fclose(file);
}