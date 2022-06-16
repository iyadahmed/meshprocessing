#include <cstring>

#include "../trimesh.hh"
#include "binary.hh"

void read_stl_binary(TriMesh &mesh, std::ifstream &ifs)
{
    const int chunk_size = 1024;
    uint32_t tris_num = 0;

    ifs.seekg(BINARY_HEADER_SIZE, std::ios_base::beg);
    if (!ifs.read(reinterpret_cast<char *>(&tris_num), sizeof(uint32_t)))
    {
        return;
    }

    STLBinaryTriangle tris_buf[chunk_size];
    Triangle mesh_tri_buf;
    size_t num_read_tris;
    while ((num_read_tris = ifs.read(reinterpret_cast<char *>(tris_buf), sizeof(STLBinaryTriangle) * chunk_size).gcount() / sizeof(STLBinaryTriangle)))
    {
        for (size_t i = 0; i < num_read_tris; i++)
        {
            memcpy(mesh_tri_buf.v1, tris_buf[i].v1, 3);
            memcpy(mesh_tri_buf.v2, tris_buf[i].v2, 3);
            memcpy(mesh_tri_buf.v3, tris_buf[i].v3, 3);
            mesh.add_triangle(mesh_tri_buf);
        }
    }
}
