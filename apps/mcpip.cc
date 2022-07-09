#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>

#include "stl_io.hh"
#include "vec3.hh"
#include "timers.hh"
#include "monte_carlo_full_sphere_samples.hh"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::Simple_cartesian<double> TreeK;

typedef TreeK::Point_3 CGALPoint3;
typedef TreeK::Triangle_3 CGALTriangle3;
typedef TreeK::Segment_3 CGALSegment3;
typedef TreeK::Vector_3 CGALVector3;
typedef TreeK::Ray_3 CGALRay3;

typedef std::vector<CGALTriangle3>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<TreeK, Iterator> Primitive;
typedef CGAL::AABB_traits<TreeK, Primitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;

using namespace mp::io::stl;

static std::vector<CGALTriangle3> load_stl(const char *filepath)
{
    std::vector<Triangle> tris;
    read_stl(filepath, tris);

    std::vector<CGALTriangle3> out;
    for (auto const &t : tris)
    {
        CGALPoint3 p1(t.v1[0], t.v1[1], t.v1[2]);
        CGALPoint3 p2(t.v2[0], t.v2[1], t.v2[2]);
        CGALPoint3 p3(t.v3[0], t.v3[1], t.v3[2]);

        out.push_back({p1, p2, p3});
    }

    return out;
}

static bool is_inside(const Tree &tree, const CGALPoint3 &&query_point)
{
    int odd_intersections_num = 0;
    int non_zero_intersections_num = 0;
    for (int i = 0; i < NUM_SPHERE_SAMPLES; i++)
    {
        CGALRay3 ray(query_point, CGALVector3{SPHERE_SAMPLES[i][0],
                                              SPHERE_SAMPLES[i][1],
                                              SPHERE_SAMPLES[i][2]});
        size_t n = tree.number_of_intersected_primitives(ray);
        non_zero_intersections_num += (bool)n;
        odd_intersections_num += (n & 1);
    }

    return (odd_intersections_num >= (.5 * float(non_zero_intersections_num)))

           && (non_zero_intersections_num >= (.4 * NUM_SPHERE_SAMPLES));
}

static bool is_inside_no_holes(const Tree &tree, const CGALPoint3 &query_point)
{
    for (int i = 0; i < NUM_SPHERE_SAMPLES; i++)
    {
        CGALRay3 ray(query_point, CGALVector3{SPHERE_SAMPLES[i][0],
                                              SPHERE_SAMPLES[i][1],
                                              SPHERE_SAMPLES[i][2]});
        if (!tree.do_intersect(ray))
        {
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        puts("Monte Carlo Point in Polygon 3D\n"
             "Usage: mcpip grid_step input_filepath.stl output_filepath.pts\n"
             "Generates points inside the volume of an oriented triangle soup by filtering bounding box grid points.\n"
             "Outputs a binary file containing N * 3 doubles.");
        return 1;
    }

    float grid_step = atof(argv[1]);
    if (grid_step <= 0.0f)
    {
        puts("ERROR: Grid step must be a positive number.");
        return 1;
    }
    char *input_filepath = argv[2];
    char *output_filepath = argv[3];

    std::cout << "CGAL Version: " << CGAL_VERSION_STR << std::endl;

    auto tri_soup = load_stl(input_filepath);
    Tree tree(tri_soup.begin(), tri_soup.end());

    // Calculate bounding box
    Vec3 bb_min(INFINITY, INFINITY, INFINITY);
    Vec3 bb_max(-INFINITY, -INFINITY, -INFINITY);
    for (auto const &tri : tri_soup)
    {
        for (int i = 0; i < 3; i++)
        {
            auto v = tri.vertex(i);
            auto vec = Vec3(v[0], v[1], v[2]);
            bb_min.min(vec);
            bb_max.max(vec);
        }
    }

    // Generate and filter grid points and write them to a file
    std::ofstream file(output_filepath, std::ios::binary);

    Vec3 bb_dims = bb_max - bb_min;
    int num_x = static_cast<int>(bb_dims.x / grid_step);
    int num_y = static_cast<int>(bb_dims.y / grid_step);
    int num_z = static_cast<int>(bb_dims.z / grid_step);

    int num_points = num_x * num_y * num_z;
    printf("Number of grid points before filtering = %d\n", num_points);

    Timer timer;
#pragma omp parallel for collapse(3)
    for (int i = 0; i < num_x; i++)
    {
        for (int j = 0; j < num_y; j++)
        {
            for (int k = 0; k < num_z; k++)
            {
                double x = i * grid_step + bb_min.x;
                double y = j * grid_step + bb_min.y;
                double z = k * grid_step + bb_min.z;
                if (is_inside(tree, {x, y, z}))
                {
#pragma omp critical
                    {
                        file.write(reinterpret_cast<const char *>(&x), sizeof(double));
                        file.write(reinterpret_cast<const char *>(&y), sizeof(double));
                        file.write(reinterpret_cast<const char *>(&z), sizeof(double));
                    }
                }
            }
        }
    }
    timer.tock("Filtering points");

    return 0;
}
