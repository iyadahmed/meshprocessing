#include <chrono>
#include <cmath>
#include <iostream>

#include "importer.hh"
#include "vec3.hh"
#include "trimesh.hh"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        puts("Usage: test_stl path_to_stl.stl");
        return 1;
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    TriMesh mesh(0);
    mp::io::stl::read_stl(mesh, argv[1]);
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time = t1 - t0;
    std::cout << "Import finished in " << time.count() << " seconds" << std::endl;

    Vec3 bb_min(INFINITY, INFINITY, INFINITY);
    Vec3 bb_max(-INFINITY, -INFINITY, -INFINITY);
    Vec3 mean(0.0, 0.0, 0.0);

    for (int i = 0; i < mesh.tris_count(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            auto vec = Vec3(mesh.get_tri(i).verts[j]);
            Vec3::min(bb_min, bb_min, vec);
            Vec3::max(bb_max, bb_max, vec);
            mean += vec / (mesh.tris_count() * 3);
        }
    }

    std::cout << "Number of Triangles = " << mesh.tris_count() << std::endl;
    std::cout << "Min: " << bb_min << std::endl;
    std::cout << "Max: " << bb_max << std::endl;
    std::cout << "Mean: " << mean << std::endl;

    mesh.free();

    return 0;
}
