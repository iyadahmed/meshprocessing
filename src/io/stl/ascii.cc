#include <fstream>

#include "string_buffer.hh"
#include "trimesh.hh"

namespace mp::io::stl
{
    void read_stl_ascii(TriMesh &mesh, std::ifstream &ifs)
    {
        std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        StringBuffer str_buf(&str[0], str.size());

        const int num_reserved_tris = 1024;
        mesh.reserve(num_reserved_tris);

        Triangle tri_buf;
        str_buf.drop_line(); /* Skip header line */
        while (!str_buf.is_empty())
        {
            if (str_buf.parse_token("vertex", 6))
            {
                str_buf.parse_float3(tri_buf.v1);
                if (str_buf.parse_token("vertex", 6))
                {
                    str_buf.parse_float3(tri_buf.v2);
                }
                if (str_buf.parse_token("vertex", 6))
                {
                    str_buf.parse_float3(tri_buf.v3);
                }
                mesh.add_triangle(tri_buf);
            }
            // else if (str_buf.parse_token("facet", 5))
            // {
            //     str_buf.drop_token(); /* Expecting "normal" */
            //     parse_float3(str_buf, custom_normal_buf);
            // }
            else
            {
                str_buf.drop_token();
            }
        }
    }
}
