#pragma once

#include <vector>

namespace mp::io::stl
{
    union Triangle
    {
        struct
        {
            float v1[3], v2[3], v3[3];
        };
        float verts[3][3];
    };
    void read_stl(const char *filepath, std::vector<Triangle> &tris);
}
