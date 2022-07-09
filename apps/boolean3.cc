#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <fstream>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Surface_mesh<K::Point_3> Mesh;
typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;
namespace PMP = CGAL::Polygon_mesh_processing;

#include "stl_io.hh"
#include "timers.hh"

using namespace mp::io;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: boolean3 a.stl b.stl" << std::endl;
        return 1;
    }

    const char *filename_1 = argv[1];
    const char *filename_2 = argv[2];

    Mesh mesh;

    std::vector<stl::Triangle> tris;
    stl::read_stl(filename_1, tris);
    stl::read_stl(filename_2, tris);
    for (const auto &t : tris)
    {
        mesh.add_face(mesh.add_vertex({t.verts[0][0], t.verts[0][1], t.verts[0][2]}),
                      mesh.add_vertex({t.verts[1][0], t.verts[1][1], t.verts[1][2]}),
                      mesh.add_vertex({t.verts[2][0], t.verts[2][1], t.verts[2][2]}));
    }

    // if (!CGAL::is_triangle_mesh(mesh))
    // {
    //     std::cerr << "Mesh contains non triangle faces." << std::endl;
    //     return 1;
    // }

    bool intersecting = PMP::does_self_intersect(mesh,
                                                 PMP::parameters::vertex_point_map(get(CGAL::vertex_point, mesh)));
    std::cout
        << (intersecting ? "There are self-intersections." : "There is no self-intersection.")
        << std::endl;
    std::vector<std::pair<face_descriptor, face_descriptor>> intersected_tris;

    PMP::self_intersections(mesh, std::back_inserter(intersected_tris));
    std::cout << intersected_tris.size() << " pairs of triangles intersect." << std::endl;

    return 0;
}
