#include "stl_importer_ascii.hh"
#include "../../string_buffer.hh"

namespace mp::io::stl {
void read_stl_ascii(std::ifstream &ifs, std::vector<Triangle> &tris) {
  std::string str((std::istreambuf_iterator<char>(ifs)),
                  std::istreambuf_iterator<char>());
  StringBuffer str_buf(&str[0], str.size());

  /* Reserve some amount of triangles to speed up things */
  tris.reserve(1024);

  Triangle tri_buf{};
  str_buf.drop_line(); /* Skip header line */
  while (!str_buf.is_empty()) {
    if (str_buf.parse_token("vertex", 6)) {
      str_buf.parse_float3(tri_buf.verts[0]);
      if (str_buf.parse_token("vertex", 6)) {
        str_buf.parse_float3(tri_buf.verts[1]);
      }
      if (str_buf.parse_token("vertex", 6)) {
        str_buf.parse_float3(tri_buf.verts[2]);
      }
      tris.push_back(tri_buf);
    }
    // else if (str_buf.parse_token("facet", 5))
    // {
    //     str_buf.drop_token(); /* Expecting "normal" */
    //     parse_float3(str_buf, custom_normal_buf);
    // }
    else {
      str_buf.drop_token();
    }
  }
}
} // namespace mp::io::stl
