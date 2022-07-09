#include <embree3/rtcore.h>
#include <vector>
#include <iostream>
#include <limits>

#include "stl_io.hh"
#include "vec3.hh"
#include "minimal_embree_bvh_build.hh"

using namespace mp::io;

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
    RTCBVH bvh = rtcNewBVH(device);

    std::vector<stl::Triangle> tri_soup;
    stl::read_stl(filepath_1, tri_soup);
    stl::read_stl(filepath_2, tri_soup);

    std::vector<RTCBuildPrimitive> primitives(tri_soup.size());

    for (int i = 0; i < tri_soup.size(); i++)
    {
        const stl::Triangle &t = tri_soup[i];
        Vec3 bb_lower(std::numeric_limits<float>::infinity());
        Vec3 bb_upper(-std::numeric_limits<float>::infinity());
        for (int j = 0; j < 3; j++)
        {
            bb_lower.min(t.verts[j]);
            bb_upper.max(t.verts[j]);
        }
        RTCBuildPrimitive &prim = primitives[i];
        prim.lower_x = bb_lower.x;
        prim.lower_y = bb_lower.y;
        prim.lower_z = bb_lower.z;
        prim.geomID = 0; // TODO: use geom id to filter geometry later based on boolean operation
        prim.upper_x = bb_upper.x;
        prim.upper_y = bb_upper.y;
        prim.upper_z = bb_upper.z;
        prim.primID = (unsigned)i;
    }

    /* settings for BVH build */
    RTCBuildArguments arguments = rtcDefaultBuildArguments();
    arguments.byteSize = sizeof(arguments);
    arguments.buildFlags = RTC_BUILD_FLAG_DYNAMIC;
    arguments.buildQuality = RTC_BUILD_QUALITY_HIGH;
    arguments.maxBranchingFactor = 2;
    arguments.maxDepth = 1024;
    arguments.sahBlockSize = 1;
    arguments.minLeafSize = 1;
    arguments.maxLeafSize = 1;
    arguments.traversalCost = 1.0f;
    arguments.intersectionCost = 1.0f;
    arguments.bvh = bvh;
    arguments.primitives = primitives.data();
    arguments.primitiveCount = primitives.size();
    arguments.primitiveArrayCapacity = primitives.capacity();
    arguments.createNode = InnerNode::create;
    arguments.setNodeChildren = InnerNode::setChildren;
    arguments.setNodeBounds = InnerNode::setBounds;
    arguments.createLeaf = LeafNode::create;
    arguments.splitPrimitive = splitPrimitive;
    arguments.buildProgress = buildProgress;
    arguments.userPtr = nullptr;

    Node *root = (Node *)rtcBuildBVH(&arguments);
    rtcReleaseBVH(bvh);

    return 0;
}
