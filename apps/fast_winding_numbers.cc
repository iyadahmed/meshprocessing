#include <Eigen/Geometry>
#include <cstdlib>
#include <igl/barycenter.h>
#include <igl/bounding_box_diagonal.h>
#include <igl/copyleft/cgal/point_areas.h>
#include <igl/fast_winding_number.h>
#include <igl/get_seconds.h>
#include <igl/knn.h>
#include <igl/octree.h>
#include <igl/per_face_normals.h>
#include <igl/random_points_on_mesh.h>
#include <igl/read_triangle_mesh.h>
#include <igl/slice_mask.h>
#include <iostream>

#include "timers.hh"

int main(int argc, char *argv[]) {
  if (argc != 5) {
    puts("Monte Carlo Point in Polygon 3D\n"
         "Usage: mcpip_embree input_filepath.stl output_filepath.pts "
         "num_points threshold\n"
         "Generates points inside the volume of an oriented triangle soup by "
         "filtering bounding box grid points.\n"
         "Outputs a binary file containing N * 3 floats.");
    return 1;
  }

  char *input_filepath = argv[1];
  char *output_filepath = argv[2];
  int num_points = atoi(argv[3]);
  float threshold = atof(argv[4]);
  if ((threshold > 1.0f) || (threshold < 0.0f)) {
    puts("ERROR: Threshold must be between 0.0 and 1.0 inclusive.");
    return 1;
  }

  Eigen::MatrixXd V;
  Eigen::MatrixXi F;
  igl::read_triangle_mesh(input_filepath, V, F);
  igl::FastWindingNumberBVH fwn_bvh;

  // igl::fast_winding_number(fwn_bvh, 2.0, V);
  igl::fast_winding_number(V.cast<float>().eval(), F, 2, fwn_bvh);

  // Generate a list of random query points in the bounding box
  Eigen::MatrixXd Q = Eigen::MatrixXd::Random(num_points, 3);
  const Eigen::RowVector3d Vmin = V.colwise().minCoeff();
  const Eigen::RowVector3d Vmax = V.colwise().maxCoeff();
  const Eigen::RowVector3d Vdiag = Vmax - Vmin;
  for (int q = 0; q < Q.rows(); q++) {
    Q.row(q) = (Q.row(q).array() * 0.5 + 0.5) * Vdiag.array() + Vmin.array();
  }

  Eigen::MatrixXd QiV;
  Eigen::VectorXf WiV;
  Timer timer;
  igl::fast_winding_number(fwn_bvh, 2, Q.cast<float>().eval(), WiV);
  timer.tock("Filtering points");
  igl::slice_mask(Q, WiV.array() > threshold, 1, QiV);

  std::cout << V.size() << std::endl;
  std::cout << F.size() << std::endl;
  std::cout << fwn_bvh.F.size() << std::endl;
  std::cout << QiV.rows() << std::endl;
  std::cout << QiV.cols() << std::endl;

  std::ofstream file(output_filepath, std::ios::binary);
  for (int i = 0; i < QiV.rows(); i++) {
    float x = QiV(i, 0);
    float y = QiV(i, 1);
    float z = QiV(i, 2);
    file.write(reinterpret_cast<const char *>(&x), sizeof(float));
    file.write(reinterpret_cast<const char *>(&y), sizeof(float));
    file.write(reinterpret_cast<const char *>(&z), sizeof(float));
  }
}