#include <vector>
#include <iostream>

#include "stl_io.hh"
#include "bvh.hh"

using namespace mp::io;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        puts("Usage: bvh4 input.stl");
        return 1;
    }

    std::vector<stl::Triangle> tris;
    stl::read_stl(argv[1], tris);

    if (tris.size() == 0)
    {
        puts("Empty mesh");
        return 0;
    }

    BVH bvh(tris);
    std::cout << bvh.count() << std::endl;

    return 0;
}