#pragma once

#include <fstream>

#include "../trimesh.hh"

namespace mp::io::stl
{
    const size_t BINARY_HEADER_SIZE = 80;
    const size_t BINARY_STRIDE = 12 * 4 + 2;

#pragma pack(push, 1)
    struct STLBinaryTriangle
    {
        float custom_normal[3];
        float v1[3], v2[3], v3[3];
        uint16_t attribute_byte_count;
    };
#pragma pack(pop)

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

    void read_stl_binary(TriMesh &mesh, std::ifstream &ifs);
}
