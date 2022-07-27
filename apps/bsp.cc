#include <iostream>
#include <fstream>
#include <vector>
#include <array>

#include "bsp.hh"
#include "stl_io.hh"

using namespace mp::io;

using IndexedTriangle = std::array<size_t, 3>;
using float3 = std::array<float, 3>;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    puts("Usage BSP input.stl");
    return 1;
  }

  std::vector<stl::Triangle> tris;
  stl::read_stl(argv[1], tris);
  std::cout << "Number of triangles = " << tris.size() << std::endl;

  std::vector<float3> points;

  std::vector<IndexedTriangle> indexed_tris;

  BSPTree tree(tris.size() * 3);
  for (const auto &t : tris) {
    IndexedTriangle it;
    for (int i = 0; i < 3; i++) {
      it[i] = tree.insert(t.verts[i][0], t.verts[i][1], t.verts[i][2]);
    }
	indexed_tris.push_back(it);
  }

  std::ofstream file("points.pts", std::ios::binary);
  for (size_t i = 0; i < tree.num; i++)
  {
	file.write((char *)&tree.nodes[i].x, sizeof(float));
	file.write((char *)&tree.nodes[i].y, sizeof(float));
	file.write((char *)&tree.nodes[i].z, sizeof(float));
  }

  std::cout << "Number of de-duplicated vertices = " << tree.num << std::endl;
}
