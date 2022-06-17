#pragma once

#include "../trimesh.hh"

namespace mp::io::stl
{
    void read_stl(TriMesh &mesh, const char *filepath);
}
