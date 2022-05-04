#pragma once

#include "../vec3.hh"
#include "../mesh.hh"

static inline long calc_file_size(FILE *file)
{
    // TODO: error checking for fseek and ftell
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return file_size;
}

void read_stl(TriMesh &mesh, const char *filepath);
