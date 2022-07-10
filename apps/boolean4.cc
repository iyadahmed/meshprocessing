#include <embree3/rtcore.h>
#include <vector>
#include <iostream>
#include <limits>

#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
// Use exact predicates and constructions to avoid precondition exception (degenerate edges being generated while intersecting triangles)
// Also for better precision and handling coplanar cases
typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef K::Point_3 Point;
typedef K::Triangle_3 Triangle;
typedef K::Segment_3 Segment;
typedef K::Vector_3 Vector;

#include "stl_io.hh"

using namespace mp::io;

/* Self-intersection data */
struct Data
{
    std::vector<stl::Triangle> tri_soup;
};

bool intersect_triangle_triangle(const std::vector<stl::Triangle> &tri_soup, unsigned geomID0, unsigned primID0, unsigned geomID1, unsigned primID1)
{
    if (primID0 == primID1)
    {
        return false;
    }

    const stl::Triangle &t1 = tri_soup[primID0];
    const stl::Triangle &t2 = tri_soup[primID1];
    Triangle cgal_t1, cgal_t2;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            cgal_t1.vertex(i)[j] = t1.verts[i][j];
            cgal_t2.vertex(i)[j] = t2.verts[i][j];
        }
    }
    return CGAL::do_intersect(cgal_t1, cgal_t2);
}

static void collide_func(void *user_data_ptr, RTCCollision *collisions, unsigned int num_collisions)
{
    for (size_t i = 0; i < num_collisions;)
    {
        bool intersect = intersect_triangle_triangle(((Data *)user_data_ptr)->tri_soup,
                                                     collisions[i].geomID0, collisions[i].primID0,
                                                     collisions[i].geomID1, collisions[i].primID1);
        if (intersect)
            i++;
        else
            collisions[i] = collisions[--num_collisions];
    }

    if (num_collisions == 0)
        return;

    // TODO: collect intersections
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: boolean3 a.stl b.stl" << std::endl;
        return 1;
    }

    const char *filepath_1 = argv[1];
    const char *filepath_2 = argv[2];

    RTCDevice device = rtcNewDevice(NULL);
    RTCScene scene = rtcNewScene(device);

    // Data MUST be allocated on heap to be shared between threads
    Data *data_ptr = new Data{};

    stl::read_stl(filepath_1, data_ptr->tri_soup);
    stl::read_stl(filepath_2, data_ptr->tri_soup);

    // TODO: use user defined geometry as rtcCollide only works with that
    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
    float *vertices = (float *)rtcSetNewGeometryBuffer(geom,
                                                       RTC_BUFFER_TYPE_VERTEX,
                                                       0,
                                                       RTC_FORMAT_FLOAT3,
                                                       3 * sizeof(float),
                                                       data_ptr->tri_soup.size() * 3);

    unsigned *indices = (unsigned *)rtcSetNewGeometryBuffer(geom,
                                                            RTC_BUFFER_TYPE_INDEX,
                                                            0,
                                                            RTC_FORMAT_UINT3,
                                                            3 * sizeof(unsigned),
                                                            data_ptr->tri_soup.size());

    if (!(vertices && indices))
    {
        std::cerr << "Failed to create Embree geometry." << std::endl;
        return 1;
    }

    int vi = 0;
    int ii = 0;
    for (const auto &t : data_ptr->tri_soup)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                vertices[vi] = t.verts[i][j];
                vi += 1;
            }
            indices[ii] = ii;
            ii += 1;
        }
    }

    /*
     * You must commit geometry objects when you are done setting them up,
     * or you will not get any intersections.
     */
    rtcCommitGeometry(geom);

    /*
     * In rtcAttachGeometry(...), the scene takes ownership of the geom
     * by increasing its reference count. This means that we don't have
     * to hold on to the geom handle, and may release it. The geom object
     * will be released automatically when the scene is destroyed.
     *
     * rtcAttachGeometry() returns a geometry ID. We could use this to
     * identify intersected objects later on.
     */
    rtcAttachGeometry(scene, geom);
    rtcReleaseGeometry(geom);

    /*
     * Like geometry objects, scenes must be committed. This lets
     * Embree know that it may start building an acceleration structure.
     */
    rtcCommitScene(scene);

    // Perform self intersection
    rtcCollide(scene, scene, collide_func, data_ptr);

    rtcReleaseScene(scene);
    rtcReleaseDevice(device);

    return 0;
}
