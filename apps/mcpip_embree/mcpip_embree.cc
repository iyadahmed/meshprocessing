#include <algorithm>
#include <cmath>
#include <cstdio>
#include <embree3/rtcore.h>
#include <execution>
#include <fstream>
#include <igl/parallel_for.h>
#include <iostream>
#include <mutex>
#include <vector>

#include "stl_io.hh"
#include "timers.hh"
#include "vec3.hh"

#include "../random_ray_directions.hh"
#include "embree_device.hh"
#include "embree_do_intersect.hh"
#include "embree_num_intersections.hh"
#include "embree_scene.hh"

using namespace mp::io::stl;

static bool is_inside(const RTCScene &scene, const float &x, const float &y,
                      const float &z, const float &threshold) {
  int odd_intersections_num = 0;
  for (int i = 0; i < NUM_RANDOM_RAY_DIRECTIONS; i++) {
    int n = num_intersections(scene, x, y, z, RANDOM_RAY_DIRECTIONS[i][0],
                              RANDOM_RAY_DIRECTIONS[i][1], RANDOM_RAY_DIRECTIONS[i][2]);
    odd_intersections_num += (n & 1); // if odd add 1, if even add 0
  }
  return (odd_intersections_num) >= (threshold * NUM_RANDOM_RAY_DIRECTIONS);
}

struct int3 {
  int x, y, z;
};

static inline int3 jagged_index(int flat_index, int num_x, int num_y,
                                int num_z) {
  int z = flat_index / (num_x * num_y);
  int x_p_y_t_num_x = flat_index - z * (num_x * num_y);
  int y = x_p_y_t_num_x / num_x;
  int x = x_p_y_t_num_x - (y * num_x);
  return {x, y, z};
}

int main(int argc, char **argv) {
  if (argc != 5) {
    puts("Monte Carlo Point in Polygon 3D\n"
         "Usage: mcpip_embree input_filepath.stl output_filepath.pts grid_step "
         "threshold\n"
         "Generates points inside the volume of an oriented triangle soup by "
         "filtering bounding box grid points.\n"
         "Outputs a binary file containing N * 3 floats.");
    return 1;
  }

  char *input_filepath = argv[1];
  char *output_filepath = argv[2];
  float grid_step = atof(argv[3]);
  if (grid_step <= 0.0f) {
    puts("ERROR: Grid step must be a positive number.");
    return 1;
  }
  float threshold = atof(argv[4]);
  if ((threshold > 1.0f) || (threshold < 0.0f)) {
    puts("ERROR: Threshold must be between 0.0 and 1.0 inclusive.");
    return 1;
  }

  std::vector<Triangle> tris;
  read_stl(input_filepath, tris);

  RTCDevice device = initializeDevice();
  RTCScene scene = initializeScene(device, tris);

  // Calculate bounding box
  Vec3 bb_min(INFINITY, INFINITY, INFINITY);
  Vec3 bb_max(-INFINITY, -INFINITY, -INFINITY);
  for (auto const &tri : tris) {
    for (int i = 0; i < 3; i++) {
      auto v = tri.verts[i];
      auto vec = Vec3(v[0], v[1], v[2]);
      bb_min.min(vec);
      bb_max.max(vec);
    }
  }

  // Generate grid points filter them and write inside points to a file
  Vec3 bb_dims = bb_max - bb_min;
  int num_x = std::ceil(bb_dims.x / grid_step);
  int num_y = std::ceil(bb_dims.y / grid_step);
  int num_z = std::ceil(bb_dims.z / grid_step);
  int num_points = num_x * num_y * num_z;
  printf("Number of grid points before filtering = %d\n", num_points);

  std::mutex mutex;
  std::ofstream file(output_filepath, std::ios::binary);
  auto func_igl = [&](int flat_index) {
    auto [i, j, k] = jagged_index(flat_index, num_x, num_y, num_z);
    float x = i * grid_step + bb_min.x;
    float y = j * grid_step + bb_min.y;
    float z = k * grid_step + bb_min.z;
    bool b = is_inside(scene, x, y, z, threshold);
    {
      std::scoped_lock lock(mutex);
      if (b) {
        file.write((char *)(&x), sizeof(float));
        file.write((char *)(&y), sizeof(float));
        file.write((char *)(&z), sizeof(float));
      }
    }
  };

  Timer timer;
  igl::parallel_for(num_points, func_igl, 1000);
  timer.tock("Filtering points");

  rtcReleaseScene(scene);
  rtcReleaseDevice(device);

  return 0;
}
