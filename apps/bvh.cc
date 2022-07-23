#include <iostream>
#include <vector>

#include "bvh.hh"
#include "stl_io.hh"
#include "timers.hh"

using namespace mp::io;

int main(int argc, char **argv) {
  if (argc != 2) {
    puts("Usage: bvh input.stl");
    return 1;
  }

  std::vector<stl::Triangle> tris_stl;
  stl::read_stl(argv[1], tris_stl);

  if (tris_stl.size() == 0) {
    puts("Empty mesh");
    return 0;
  }

  std::vector<BVHTriangle> input_tris;
  for (const auto &t : tris_stl)
  {
    input_tris.push_back({t.verts[0], t.verts[1], t.verts[2]});
  }

  Timer t;
  BVH bvh(input_tris);
  t.tock();
  std::cout << bvh.count() << std::endl;

  return 0;
}