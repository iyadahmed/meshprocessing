#include <vector>
#include <cstdio>
#include <limits>
#include <execution>
#include <iostream>

#include "stl_io.hh"
#include "vec3.hh"
#include "timers.hh"

using namespace mp::io;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        puts("Usage: boolean4 a.stl b.stl");
        return 1;
    }

    const char *filepath_1 = argv[1];
    const char *filepath_2 = argv[2];

    std::vector<stl::Triangle> tri_soup;
    stl::read_stl(filepath_1, tri_soup);
    stl::read_stl(filepath_2, tri_soup);

    Vec3 bb_min(INFINITY);
    Vec3 bb_max(-INFINITY);
    Vec3 bb_dims_avg(0.0f);

    Vec3 t_bb_min(INFINITY);
    Vec3 t_bb_max(-INFINITY);

    Timer timer;
    for (auto const &tri : tri_soup)
    {
        t_bb_min = INFINITY;
        t_bb_max = -INFINITY;
        for (int i = 0; i < 3; i++)
        {
            bb_min.min(tri.verts[i]);
            bb_max.max(tri.verts[i]);

            t_bb_min.min(tri.verts[i]);
            t_bb_max.max(tri.verts[i]);
        }
        bb_dims_avg += (t_bb_max - t_bb_min) / tri_soup.size();
    }
    timer.tock("First Pass");

    // TODO: dynamic grid step
    float grid_step = .3;
    Vec3 bb_dims = bb_max - bb_min;
    int num_x = std::ceil(bb_dims.x / grid_step);
    int num_y = std::ceil(bb_dims.y / grid_step);
    int num_z = std::ceil(bb_dims.z / grid_step);

    std::vector<std::vector<int>> cells(num_x * num_y * num_z);

    timer.tick();
    for (int i = 0; i < tri_soup.size(); i++)
    {
        const auto &t = tri_soup[i];
        t_bb_min = INFINITY;
        t_bb_max = -INFINITY;
        for (int i = 0; i < 3; i++)
        {
            t_bb_min.min(t.verts[i]);
            t_bb_max.max(t.verts[i]);
        }
        for (int x = std::ceil(t_bb_min.x / grid_step); x < std::ceil(t_bb_max.x / grid_step); x++)
        {
            for (int y = std::ceil(t_bb_min.y / grid_step); y < std::ceil(t_bb_max.y / grid_step); y++)
            {
                for (int z = std::ceil(t_bb_min.z / grid_step); z < std::ceil(t_bb_max.z / grid_step); z++)
                {
                    int cell_index = x + y * num_y + z * (num_x * num_y);
                    cells[cell_index].push_back(i);
                }
            }
        }
    }
    timer.tock("Second Pass");

    std::cout << "Avg. Triangle BB = " << bb_dims_avg << std::endl;
    std::cout << "Mesh Bounding Box: Max(" << bb_max << "), Min(" << bb_min << ")" << std::endl;
    std::cout << "Cells Size " << cells.size() << std::endl;
    return 0;
}
