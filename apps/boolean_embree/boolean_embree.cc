#include <embree3/rtcore.h>
#include <execution>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

#include "boolean_embree.hh"
#include "stl_io.hh"
#include "timers.hh"
#include "utils.hh"

using namespace mp::io;

static inline void write_point(std::ofstream &file, const Point &p) {
  double x = CGAL::to_double(p.x());
  double y = CGAL::to_double(p.y());
  double z = CGAL::to_double(p.z());
  file.write((char *)(&x), sizeof(double));
  file.write((char *)(&y), sizeof(double));
  file.write((char *)(&z), sizeof(double));
}

static inline stl::Triangle
to_stl_triangle(Triangulation::Finite_faces_iterator &it) {
  stl::Triangle out;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      out.verts[i][j] = CGAL::to_double(it->vertex(i)->info().point_3d[j]);
    }
  }
  return out;
}

static inline stl::Triangle to_stl_triangle(const Triangle &t) {
  stl::Triangle out;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      out.verts[i][j] = CGAL::to_double(t.vertex(i)[j]);
    }
  }
  return out;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "Usage: boolean_embree a.stl b.stl" << std::endl;
    return 1;
  }

  const char *filepath_1 = argv[1];
  const char *filepath_2 = argv[2];

  std::vector<stl::Triangle> input_tris;
  stl::read_stl(filepath_1, input_tris);
  stl::read_stl(filepath_2, input_tris);
  std::cout << "Number of Input Triangles = " << input_tris.size() << std::endl;

  // Data MUST be allocated on heap to be shared between threads
  IntersectionData *data = new IntersectionData;

  for (const auto &t : input_tris) {
    Vec3 *v = (Vec3 *)t.verts;
    if (!is_degenerate(v[0], v[1], v[2], .00001f)) {
      data->input_tris.push_back({v[0], v[1], v[2]});
    }
  }

  RTCDevice device = rtcNewDevice(NULL);
  RTCScene scene = rtcNewScene(device);
  RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_USER);
  unsigned int geomID = rtcAttachGeometry(scene, geom);
  rtcSetGeometryUserPrimitiveCount(geom, data->input_tris.size());
  rtcSetGeometryUserData(geom, data);
  rtcSetGeometryBoundsFunction(geom, triangle_bounds_func, nullptr);
  // TODO: set ray triangle intersection function
  // rtcSetGeometryIntersectFunction(geom, triangle_intersect_func);
  rtcCommitGeometry(geom);
  rtcReleaseGeometry(geom);

  // Like geometry objects, scenes must be committed. This lets
  // Embree know that it may start building an acceleration structure.
  rtcCommitScene(scene);

  // Self intersection
  Timer timer;
  rtcCollide(scene, scene, collide_func, data);
  timer.tock("Calculating self-intersection points");

  // Triangulation
  // timer.tick();
  // std::vector<stl::Triangle> out;
  // stl::Triangle tri_buf;
  // std::vector<bool> is_triangulated(data->cgal_tris.size(), false);
  // for (auto &prim_id_intersection_points_pair :
  // data->intersection_points_map) { unsigned int tri_index =
  // prim_id_intersection_points_pair.first; auto &triangulation_input =
  // prim_id_intersection_points_pair.second;

  // is_triangulated[tri_index] = true;

  // const auto &t = data->cgal_tris[tri_index].t;
  // Include original triangle points in the triangulation
  // not just intersecion points
  // for (int i = 0; i < 3; i++) {
  // auto p = t.vertex(i);
  // TriangulationPointInfo info{p};
  // auto p2d = project_point(p, t);
  // triangulation_input.push_back({p2d, info});
  //}
  // Triangulation triangulation(triangulation_input.begin(),
  //                          triangulation_input.end());
  // for (auto it = triangulation.finite_faces_begin();
  //  it != triangulation.finite_faces_end(); it++) {
  // out.push_back(to_stl_triangle(it));
  // }
  //}
  // timer.tock("Triangulation");

  // for (size_t i = 0; i < data->cgal_tris.size(); i++) {
  // if (!is_triangulated[i]) {
  //   out.push_back(to_stl_triangle(data->cgal_tris[i].t));
  //  }
  //}

  // std::cout << "Number of output triangles = " << out.size() << std::endl;
  // stl::write_stl(out, "boolean_embree_output.stl");

  rtcReleaseScene(scene);
  rtcReleaseDevice(device);
  delete data;

  return 0;
}
