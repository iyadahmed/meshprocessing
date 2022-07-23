#include <cstdint>
#include <fstream>

#include "stl_exporter_binary.hh"

namespace mp::io::stl {
void write_stl_binary(const std::vector<Triangle> &tris, const char *filepath) {
  char header[80]{};
  uint32_t tris_num = tris.size();
  float normal_buffer[3] = {0.0f, 0.0f, 0.0f};
  uint16_t attribute_byte_count = 0;

  std::ofstream ofs(filepath, std::ios::binary);

  ofs.write(header, 80);
  ofs.write(reinterpret_cast<char *>(&tris_num), sizeof(uint32_t));

  for (auto const &t : tris) {
    ofs.write(reinterpret_cast<char *>(normal_buffer), sizeof(float[3]));
    ofs.write(reinterpret_cast<const char *>(t.verts), sizeof(float[3][3]));
    ofs.write(reinterpret_cast<char *>(&attribute_byte_count),
              sizeof(uint16_t));
  }
}
} // namespace mp::io::stl
