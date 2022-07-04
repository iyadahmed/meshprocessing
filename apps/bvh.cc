#include <cstdio>
#include <vector>
#include <iostream>

#include "stl_io.hh"
#include "vec3.hh"

class float3
{
public:
    float x, y, z;
    float3()
    {
        x = y = z = 0.0;
    }
    float3(float v)
    {
        x = y = z = v;
    }
    float3(float x, float y, float z) : x(x), y(y), z(z)
    {
    }
    float3 operator-(const float3 &other) const
    {
        return float3{x - other.x, y - other.y, z - other.z};
    }
    float3 operator+(const float3 &other) const
    {
        return float3{x + other.x, y + other.y, z + other.z};
    }
    float3 operator*(float scale) const
    {
        return float3{x * scale, y * scale, z * scale};
    }
};

struct Tri
{
    float3 vertex0, vertex1, vertex2;
    float3 centroid;
};

struct Ray
{
    float3 O, D;
    float t = 1e30f;
};

#define N 64

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

using namespace mp::io;

#include <random>

std::random_device rd;  // Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
std::uniform_real_distribution<> dis(0.0, 1.0);

static float RandomFloat()
{
    return dis(gen);
}

static float dot(const float3 &a, const float3 &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static float3 normalized(const float3 &v)
{
    float l = std::sqrt(dot(v, v));
    return float3(v.x / l, v.y / l, v.z / l);
}

static float3 cross(const float3 &a, const float3 &b)
{
    float3 out;
    out.x = a.y * b.z - a.z * b.y;
    out.y = a.z * b.x - a.x * b.z;
    out.z = a.x * b.y - a.y * b.x;
    return out;
}

void IntersectTri(Ray &ray, const Tri &tri)
{
    const float3 edge1 = tri.vertex1 - tri.vertex0;
    const float3 edge2 = tri.vertex2 - tri.vertex0;
    const float3 h = cross(ray.D, edge2);
    const float a = dot(edge1, h);
    if (a > -0.0001f && a < 0.0001f)
        return; // ray parallel to triangle
    const float f = 1 / a;
    const float3 s = ray.O - tri.vertex0;
    const float u = f * dot(s, h);
    if (u < 0 || u > 1)
        return;
    const float3 q = cross(s, edge1);
    const float v = f * dot(ray.D, q);
    if (v < 0 || u + v > 1)
        return;
    const float t = f * dot(edge2, q);
    if (t > 0.0001f)
        ray.t = std::min(ray.t, t);
}

int main(int argc, char **argv)
{
    // if (argc != 2)
    // {
    //     puts("Usage: bvh input.stl");
    //     return 1;
    // }

    Tri tri[N];
    for (int i = 0; i < N; i++)
    {
        float3 r0{RandomFloat(), RandomFloat(), RandomFloat()};
        float3 r1{RandomFloat(), RandomFloat(), RandomFloat()};
        float3 r2{RandomFloat(), RandomFloat(), RandomFloat()};
        tri[i].vertex0 = r0 * 9 - float3(5);
        tri[i].vertex1 = tri[i].vertex0 + r1;
        tri[i].vertex2 = tri[i].vertex0 + r2;
    }

    const int width = 640;
    const int height = 640;
    uint8_t image[width * height * 3]{};
    uint8_t *image_iter = image;

    float3 camPos(0, 0, -18);
    float3 p0(-1, 1, -15), p1(1, 1, -15), p2(-1, -1, -15);
    Ray ray;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float3 pixelPos = p0 + (p1 - p0) * (x / static_cast<float>(width)) + (p2 - p0) * (y / static_cast<float>(height));
            ray.O = camPos;
            ray.D = normalized(pixelPos - ray.O);
            ray.t = 1e30f;

            for (const auto &t : tri)
            {
                IntersectTri(ray, t);
            }

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

    write_ppm("bvh_render.ppm", image, width, height);
    return 0;
}
