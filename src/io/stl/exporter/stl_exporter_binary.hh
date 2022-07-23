#pragma once

#include <vector>

#include "stl_io.hh"

namespace mp::io::stl {
void write_stl_binary(const std::vector<Triangle> &tris, const char *filepath);
}
