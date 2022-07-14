// Finds intersections via a uniform grid,
// not memory efficient, but maybe very parallel friendly
// after looking at some nvidia GPU bvh posts
// I concoluded that it is better to go the BVH route indeed,
// this is also by intuition since bvh is sortof an optimziation over this bounding box technique

#include <vector>
#include <cstdio>
#include <limits>
#include <execution>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <cassert>

#include "stl_io.hh"
#include "vec3.hh"
#include "timers.hh"
#include "boolean5.hh"

using namespace mp::io;

struct IntersectionPair
{
    int t1_index, t2_index;
    IntersectionPair(int t1_index_i, int t2_index_i)
    {
        t1_index = t1_index_i;
        t2_index = t2_index_i;
        if (t1_index > t2_index)
        {
            std::swap(t1_index, t2_index);
        }
    }
};

bool operator==(const IntersectionPair &lhs, const IntersectionPair &rhs)
{
    return lhs.t1_index == rhs.t1_index && lhs.t2_index == rhs.t2_index;
}

template <>
struct std::hash<IntersectionPair>
{
    std::size_t operator()(IntersectionPair const &p) const noexcept
    {
        std::size_t h1 = std::hash<int>{}(p.t1_index);
        std::size_t h2 = std::hash<int>{}(p.t2_index);
        return h1 ^ (h2 << 1); // or use boost::hash_combine
    }
};

static inline Triangle to_cgal_triangle(const stl::Triangle &t)
{
    return {
        {t.v1[0], t.v1[1], t.v1[2]},
        {t.v2[0], t.v2[1], t.v2[2]},
        {t.v3[0], t.v3[1], t.v3[2]},
    };
}

struct IntBounds
{
    int start_x, start_y, start_z;
    int end_x, end_y, end_z;
};

static inline void calc_bounds(const stl::Triangle &t, Vec3 &min, Vec3 &max)
{
    Vec3 *verts = (Vec3 *)t.verts;
    min = verts[0];
    max = verts[0];
    for (int i = 1; i < 3; i++)
    {
        min.min(verts[i]);
        max.max(verts[i]);
    }
}

// Convert voxel position to voxel linear array index
int flat_index(int x, int y, int z, int num_x, int num_y, int num_z)
{
    return x + y * num_x + z * (num_x * num_y);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        puts("Usage: boolean6 a.stl b.stl");
        return 1;
    }

    const char *filepath_1 = argv[1];
    const char *filepath_2 = argv[2];

    std::vector<stl::Triangle> tri_soup;
    stl::read_stl(filepath_1, tri_soup);
    stl::read_stl(filepath_2, tri_soup);

    Vec3 bb_min(INFINITY), bb_max(-INFINITY);

    Timer timer;
    for (auto const &tri : tri_soup)
    {
        for (int i = 0; i < 3; i++)
        {
            bb_min.min(tri.verts[i]);
            bb_max.max(tri.verts[i]);
        }
        // todo: use std::accumulate to calc avg bounding box in parallel
        // bb_dims_avg += (t_bb_max - t_bb_min) / tri_soup.size();
    }
    timer.tock("First Pass");

    // TODO: dynamic grid step?
    float grid_step = 5;
    Vec3 bb_dims = bb_max - bb_min;
    int num_x = std::ceil(bb_dims.x / grid_step);
    int num_y = std::ceil(bb_dims.y / grid_step);
    int num_z = std::ceil(bb_dims.z / grid_step);

    assert(flat_index(num_x - 1, num_y - 1, num_z - 1, num_x, num_y, num_z) == (num_x * num_y * num_z - 1));

    std::vector<IntBounds> ctris(tri_soup.size());

    std::unordered_map<int, std::vector<int>> cells;
    cells.reserve(tri_soup.size());

    timer.tick();
    for (int i = 0; i < tri_soup.size(); i++)
    {
        const auto &t = tri_soup[i];
        Vec3 t_bb_min, t_bb_max;
        calc_bounds(t, t_bb_min, t_bb_max);

        // FIXME: double check logic
        // floor should be used for lower bound
        // but it causes the program to hang???
        int start_x = std::ceil((t_bb_min.x - bb_min.x) / grid_step);
        int start_y = std::ceil((t_bb_min.y - bb_min.y) / grid_step);
        int start_z = std::ceil((t_bb_min.z - bb_min.z) / grid_step);

        int end_x = std::ceil((t_bb_max.x - bb_min.x) / grid_step);
        int end_y = std::ceil((t_bb_max.y - bb_min.y) / grid_step);
        int end_z = std::ceil((t_bb_max.z - bb_min.z) / grid_step);

        assert(start_x >= 0);
        assert(start_y >= 0);
        assert(start_z >= 0);
        assert(end_x >= 0);
        assert(end_y >= 0);
        assert(end_z >= 0);

        ctris[i].start_x = start_x;
        ctris[i].start_y = start_y;
        ctris[i].start_z = start_z;

        ctris[i].end_x = end_x;
        ctris[i].end_y = end_y;
        ctris[i].end_z = end_z;

        for (int x = start_x; x < end_x; x++)
        {
            for (int y = start_y; y < end_y; y++)
            {
                for (int z = start_z; z < end_z; z++)
                {
                    int cell_index = x + y * num_x + z * (num_x * num_y);
                    cells[cell_index].push_back(i);
                }
            }
        }
    }
    timer.tock("Second Pass");

    std::unordered_set<IntersectionPair> interescetions;
    interescetions.reserve(tri_soup.size());

    timer.tick();
    for (int i = 0; i < tri_soup.size(); i++)
    {
        const auto &t = tri_soup[i];

        int start_x = ctris[i].start_x;
        int start_y = ctris[i].start_y;
        int start_z = ctris[i].start_z;

        int end_x = ctris[i].end_x;
        int end_y = ctris[i].end_y;
        int end_z = ctris[i].end_z;

        for (int x = start_x; x < end_x; x++)
        {
            for (int y = start_y; y < end_y; y++)
            {
                for (int z = start_z; z < end_z; z++)
                {
                    int cell_index = x + y * num_x + z * (num_x * num_y);
                    auto it = cells.find(cell_index);
                    if (it == cells.end())
                    {
                        continue;
                    }
                    for (const auto &other_triangle_index : it->second)
                    {
                        if (i == other_triangle_index)
                        {
                            continue;
                        }
                        if (CGAL::do_intersect(to_cgal_triangle(t), to_cgal_triangle(tri_soup[other_triangle_index])))
                        {
                            interescetions.insert({i, other_triangle_index});
                        }
                    }
                }
            }
        }
    }
    timer.tock("Third Pass");

    std::vector<Point> intersection_points;
    intersection_points.reserve(interescetions.size());

    timer.tick();
    // for (const auto &ip : interescetions)
    // {
    //     cgal_tri_tri_intersection_points(intersection_points, to_cgal_triangle(tri_soup[ip.t1_index]), to_cgal_triangle(tri_soup[ip.t2_index]));
    // }
    timer.tock("Fourth Pass");

    std::ofstream file("foo.pts", std::ios::binary);
    for (const auto &ip : intersection_points)
    {
        double x = CGAL::to_double(ip.x());
        double y = CGAL::to_double(ip.y());
        double z = CGAL::to_double(ip.z());
        file.write((char *)(&x), sizeof(double));
        file.write((char *)(&y), sizeof(double));
        file.write((char *)(&z), sizeof(double));
    }

    // TODO: intersect edges instead

    // std::cout << "Avg. Triangle BB = " << bb_dims_avg << std::endl;
    std::cout << "Mesh Bounding Box: Max(" << bb_max << "), Min(" << bb_min << ")" << std::endl;
    std::cout << "Cells Size " << cells.size() << std::endl;
    std::cout << "Intersection Size " << interescetions.size() << std::endl;
    std::cout << "Intersection Points Size " << intersection_points.size() << std::endl;
    return 0;
}
