#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>

#include "stl.hpp"

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    puts("Usage: test_stl path_to_stl.stl");
    return 1;
  }

  auto t0 = std::chrono::high_resolution_clock::now();
  Mesh mesh(0);
  read_stl(mesh, argv[1]);
  auto t1 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> time = t1 - t0;
  std::cout << "Import finished in " << time.count() << " seconds" << std::endl;

  Vec3 bb_min(INFINITY, INFINITY, INFINITY);
  Vec3 bb_max(-INFINITY, -INFINITY, -INFINITY);
  Vec3 mean(0.0, 0.0, 0.0);

  for (int i = 0; mesh.verts && (i < mesh.count); i++)
  {
    auto vec = mesh.verts[i];

    Vec3::min(bb_min, bb_min, vec);
    Vec3::max(bb_max, bb_max, vec);
    mean += vec;
  }
  mean /= mesh.count;

  std::cout << "Min: " << bb_min << std::endl;
  std::cout << "Max: " << bb_max << std::endl;
  std::cout << "Mean: " << mean << std::endl;

  mesh.free();

  return 0;
}