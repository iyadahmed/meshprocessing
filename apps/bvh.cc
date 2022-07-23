#include <iostream>
#include <vector>

#include "bvh.hh"
#include "stl_io.hh"

using namespace mp::io;

int main(int argc, char **argv) {
  if (argc != 2) {
    puts("Usage: bvh input.stl");
    return 1;
  }

  std::vector<stl::Triangle> tris;
  stl::read_stl(argv[1], tris);

  if (tris.size() == 0) {
    puts("Empty mesh");
    return 0;
  }

  BVH bvh(tris);
  std::cout << bvh.count() << std::endl;

  return 0;
}