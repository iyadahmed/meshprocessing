#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

#include "stl_exporter_binary.hh"
#include "stl_importer_ascii.hh"
#include "stl_importer_binary.hh"
#include "stl_io.hh"

namespace mp::io::stl {
void write_stl(const std::vector<Triangle> &tris, const char *filepath) {
  write_stl_binary(tris, filepath);
}

void read_stl(const char *filepath, std::vector<Triangle> &tris) {
  std::ifstream ifs(filepath, std::ios::binary);

  /* Detect STL file type by comparing file size with expected file size,
   * could check if file starts with "solid", but some files do not adhere.
   */
  uint32_t tris_num = 0;
  auto file_size = std::filesystem::file_size(filepath);
  ifs.seekg(BINARY_HEADER_SIZE, std::ios_base::beg);
  ifs.read(reinterpret_cast<char *>(&tris_num), sizeof(uint32_t));
  auto expected_binary_file_size =
      BINARY_HEADER_SIZE + 4 + BINARY_STRIDE * tris_num;
  bool is_binary_stl = (file_size == expected_binary_file_size);

  if (is_binary_stl) {
    read_stl_binary(ifs, tris);
  } else {
    read_stl_ascii(ifs, tris);
  }
}
} // namespace mp::io::stl
