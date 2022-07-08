#include <vector>

#include "bvh.hh"
#include "random2.hh"
#include "stl_io.hh"
#include "timers.hh"

#define N 64

using namespace mp::io;

/* Export image as a binary RGB PPM image, returns 0 on success, 1 on failure */
static int write_ppm(const char *filepath, const uint8_t *image, int width, int height)
{
    FILE *file = fopen(filepath, "wb");
    if (file == NULL)
    {
        puts("Error opening file");
        return 1;
    }
    fprintf(file, "P6 %d %d 255\n", width, height);
    fwrite(image, sizeof(uint8_t), width * height * 3, file);
    fclose(file);
    return 0;
}

int main(int argc, char **argv)
{
    // if (argc != 2)
    // {
    //     puts("Usage: bvhapp2 input.stl");
    //     return 1;
    // }

    // std::vector<stl::Triangle> tri_soup;
    // stl::read_stl(argv[1], tri_soup);
    // tri_soup[0];

    auto bvh = BVH(N);
    for (int i = 0; i < N; i++)
    {
        BVHTriangle &tri = bvh.triangle(i);
        float3 r0{random_float(), random_float(), random_float()};
        float3 r1{random_float(), random_float(), random_float()};
        float3 r2{random_float(), random_float(), random_float()};
        tri.v0 = r0 * 9 - float3(5);
        tri.v1 = tri.v0 + r1;
        tri.v2 = tri.v0 + r2;
    }
    {
        ScopedTimer("Building BVH");
        bvh.build();
    }

    const int width = 640;
    const int height = 640;
    uint8_t image[width * height * 3]{};
    uint8_t *image_iter = image;

    float3 camera_position(0, 0, -18);
    float3 p0(-1, 1, -15), p1(1, 1, -15), p2(-1, -1, -15);
    BVHRay ray;
    Timer timer;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float3 pixel_position = p0 + (p1 - p0) * (x / static_cast<float>(width)) + (p2 - p0) * (y / static_cast<float>(height));
            ray.O = camera_position;
            ray.D = (pixel_position - ray.O); //.normalized();
            ray.D.normalize();
            ray.t = 1e30f;

            // for (int i = 0; i < N; i++)
            // {
            //     intersect_ray_tri(ray, bvh.triangle(i));
            // }
            bvh.intersect_ray(ray, 0);

            if (ray.t != 1e30f)
            {
                image_iter[0] = 255;
                image_iter[1] = 255;
                image_iter[2] = 255;
            }
            else
            {
                image_iter[0] = 0;
                image_iter[1] = 0;
                image_iter[2] = 0;
            }

            image_iter += 3;
        }
    }
    timer.tock("Rendering");

    write_ppm("bvh_render.ppm", image, width, height);
}
