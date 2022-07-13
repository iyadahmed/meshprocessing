#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>

#include "stl_io.hh"
#include "vec3.hh"
#include "monte_carlo_full_sphere_samples.hh"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

#define PI 3.14159265359


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

static double tet_solid_angle(const CGALPoint3 &origin, const CGALPoint3 &a, const CGALPoint3 &b, const CGALPoint3 &c)
{
    auto av = a - origin;
    auto bv = b - origin;
    auto cv = c - origin;

    auto al = sqrt(av.squared_length());
    auto bl = sqrt(bv.squared_length());
    auto cl = sqrt(cv.squared_length());

    auto numerator = CGAL::determinant(av, bv, cv);
    auto av_dot_bv = CGAL::scalar_product(av, bv);
    auto av_dot_cv = CGAL::scalar_product(av, cv);
    auto bv_dot_cv = CGAL::scalar_product(bv, cv);

    auto denominator = al * bl * cl + av_dot_bv * cl + av_dot_cv * bl + bv_dot_cv * al;

    return atan2(numerator, denominator);
}

static bool is_inside(const Tree &tree, const CGALPoint3 &query_point, float hole_tolerance)
{
    double w = 0.0;
    int n = 0;
    for (int i = 0; i < NUM_SPHERE_SAMPLES; i++)
    {
        CGALRay3 ray(query_point, CGALVector3{SPHERE_SAMPLES[i][0],
                                              SPHERE_SAMPLES[i][1],
                                              SPHERE_SAMPLES[i][2]});
        auto intersected_triangle = tree.first_intersected_primitive(ray);
        if (intersected_triangle)
        {
            auto it = boost::get<Iterator>(intersected_triangle);
            w += tet_solid_angle(query_point, it->vertex(0), it->vertex(1), it->vertex(2));
            n += 1;
        }
    }

    hole_tolerance = std::clamp(hole_tolerance, 0.0f, 1.0f);
    return (w >= ((1.0f - hole_tolerance) * 2.0 * PI)) || (n == 100);
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        puts("Monte Carlo Winding Numbers\n"
             "Usage: mcwn grid_step hole_tolerance input_filepath.stl output_filepath.pts\n"
             "Example: mcwn bunny.stl 5.0 1.0 bunny_points.pts\n"
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
    float hole_tolerance = atof(argv[2]);
    if (hole_tolerance > 1.0f || hole_tolerance < 0.0f)
    {
        puts("ERROR: Hole tolerance must be between 0.0 and 1.0 inclusive.");
        return 1;
    }
    char *input_filepath = argv[3];
    char *output_filepath = argv[4];

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

#pragma omp parallel for collapse(3)
    for (int i = 0; i < num_x; i++)
    {
        for (int j = 0; j < num_y; j++)
        {
            for (int k = 0; k < num_z; k++)
            {
                CGALPoint3 query_point(i * grid_step + bb_min.x, j * grid_step + bb_min.y, k * grid_step + bb_min.z);
                if (is_inside(tree, query_point, hole_tolerance))
                {
#pragma omp critical
                    {
                        file.write(reinterpret_cast<const char *>(&query_point.x()), sizeof(double));
                        file.write(reinterpret_cast<const char *>(&query_point.y()), sizeof(double));
                        file.write(reinterpret_cast<const char *>(&query_point.z()), sizeof(double));
                    }
                }
            }
        }
    }

    return 0;
}
