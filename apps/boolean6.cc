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
#include <fstream>

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

struct CellTriangleIndexPair
{
    int cell_index, triangle_index;
};

struct CTriangle
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
    float grid_step = .3;
    Vec3 bb_dims = bb_max - bb_min;
    int num_x = std::ceil(bb_dims.x / grid_step);
    int num_y = std::ceil(bb_dims.y / grid_step);
    int num_z = std::ceil(bb_dims.z / grid_step);

    std::vector<CellTriangleIndexPair> cells(tri_soup.size());

    timer.tick();
    for (int i = 0; i < tri_soup.size(); i++)
    {
        const auto &t = tri_soup[i];
        Vec3 t_bb_min, t_bb_max;
        calc_bounds(t, t_bb_min, t_bb_max);
        // TODO: store triangle start and end x, y, z
        for (int x = std::ceil(t_bb_min.x / grid_step); x < std::ceil(t_bb_max.x / grid_step); x++)
        {
            for (int y = std::ceil(t_bb_min.y / grid_step); y < std::ceil(t_bb_max.y / grid_step); y++)
            {
                for (int z = std::ceil(t_bb_min.z / grid_step); z < std::ceil(t_bb_max.z / grid_step); z++)
                {
                    int cell_index = x + y * num_x + z * (num_x * num_y);
                    cells[i].triangle_index = i;
                    cells[i].cell_index = cell_index;
                }
            }
        }
    }
    timer.tock("Second Pass");

    // Sort by cell index
    timer.tick();
    std::sort(cells.begin(), cells.end(), [](CellTriangleIndexPair &a, CellTriangleIndexPair &b)
              { return a.cell_index < b.cell_index; });
    timer.tock("Sorting");

    std::unordered_set<IntersectionPair> interescetions;
    interescetions.reserve(tri_soup.size());

    timer.tick();
    for (int i = 0; i < tri_soup.size(); i++)
    {
        const auto &t = tri_soup[i];
        Vec3 t_bb_min, t_bb_max;
        calc_bounds(t, t_bb_min, t_bb_max);
        // FIXME:: use floor for start x, y, z and ceil for end
        for (int x = std::ceil(t_bb_min.x / grid_step); x < std::ceil(t_bb_max.x / grid_step); x++)
        {
            for (int y = std::ceil(t_bb_min.y / grid_step); y < std::ceil(t_bb_max.y / grid_step); y++)
            {
                for (int z = std::ceil(t_bb_min.z / grid_step); z < std::ceil(t_bb_max.z / grid_step); z++)
                {
                    int cell_index = x + y * num_x + z * (num_x * num_y);
                    auto lower = std::lower_bound(cells.begin(), cells.end(), cell_index, [](const CellTriangleIndexPair &pair, int value)
                                                  { return pair.cell_index < value; });
                    if (lower == cells.end())
                    {
                        continue;
                    }
                    for (auto it = lower; it < cells.end(); it++)
                    {
                        if (it->cell_index != cell_index)
                        {
                            break;
                        }
                        if (i == it->triangle_index)
                        {
                            // Skip same triangle
                            continue;
                        }
                        if (CGAL::do_intersect(to_cgal_triangle(t), to_cgal_triangle(tri_soup[it->triangle_index])))
                        {
                            interescetions.insert({i, it->triangle_index});
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
    for (const auto &ip : interescetions)
    {
        cgal_tri_tri_intersection_points(intersection_points, to_cgal_triangle(tri_soup[ip.t1_index]), to_cgal_triangle(tri_soup[ip.t2_index]));
    }
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
