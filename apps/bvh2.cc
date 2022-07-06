#include "bvh.hh"
#include "random2.hh"

#define N 64

int main()
{
    TEST();

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
    bvh.build();
}
