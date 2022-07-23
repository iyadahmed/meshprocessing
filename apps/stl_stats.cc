#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

#include "stl_io.hh"
#include "timers.hh"
#include "vec3.hh"

using namespace mp::io::stl;

int main(int argc, char **argv) {
  if (argc != 2) {
    puts("Usage: test_stl path_to_stl.stl");
    return 1;
  }

  std::vector<Triangle> tris;
  {
    ScopedTimer("Reading STL");
    read_stl(argv[1], tris);
  }

  Vec3 bb_min(INFINITY, INFINITY, INFINITY);
  Vec3 bb_max(-INFINITY, -INFINITY, -INFINITY);
  Vec3 mean(0, 0, 0);

  for (const auto &tri : tris) {
    for (int i = 0; i < 3; i++) {
      auto vec = Vec3(tri.verts[i]);
      bb_min.min(vec);
      bb_max.max(vec);
      mean += vec / (tris.size() * 3);
    }
  }

  std::cout << "Number of Triangles = " << tris.size() << std::endl;
  std::cout << "Bounding Box Min: " << Vec3(bb_min) << std::endl;
  std::cout << "Bounding Box Max: " << Vec3(bb_max) << std::endl;
  std::cout << "Mean: " << mean << std::endl;

  return 0;
}
