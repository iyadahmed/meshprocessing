#include <chrono>
#include <iostream>
#include <vector>
#include <cmath>

#include "importer.hh"
#include "vec3.hh"

static void min_f3(float out[3], const float a[3], const float b[3])
{
    for (int i = 0; i < 3; i++)
    {
        out[i] = a[i] < b[i] ? a[i] : b[i];
    }
}

static void max_f3(float out[3], const float a[3], const float b[3])
{
    for (int i = 0; i < 3; i++)
    {
        out[i] = a[i] > b[i] ? a[i] : b[i];
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        puts("Usage: test_stl path_to_stl.stl");
        return 1;
    }

    auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<mp::io::stl::Triangle> tris;
    mp::io::stl::read_stl(argv[1], tris);

    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time = t1 - t0;
    std::cout << "Import finished in " << time.count() << " seconds" << std::endl;

    float bb_min[] = {INFINITY, INFINITY, INFINITY};
    float bb_max[] = {-INFINITY, -INFINITY, -INFINITY};
    Vec3 mean(0, 0, 0);

    for (auto const &tri : tris)
    {
        for (int i = 0; i < 3; i++)
        {
            auto vec = Vec3(tri.verts[i]);
            min_f3(bb_min, bb_min, tri.verts[i]);
            max_f3(bb_max, bb_max, tri.verts[i]);
            mean += vec / (tris.size() * 3);
        }
    }

    std::cout << "Number of Triangles = " << tris.size() << std::endl;
    std::cout << "Bounding Box Min: " << Vec3(bb_min) << std::endl;
    std::cout << "Bounding Box Max: " << Vec3(bb_max) << std::endl;
    std::cout << "Mean: " << mean << std::endl;

    return 0;
}
