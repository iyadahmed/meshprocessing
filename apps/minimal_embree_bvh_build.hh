#include <embree3/rtcore.h>
#include <cassert>
#include <limits>

#include "vec3.hh"

struct BBox3fa
{
    Vec3 upper = -std::numeric_limits<float>::infinity();
    Vec3 lower = std::numeric_limits<float>::infinity();
};

float area(const BBox3fa &bbox)
{
    Vec3 dims = bbox.upper - bbox.lower;
    return 2.0f * (dims[0] * dims[1] + dims[0] * dims[2] + dims[2] * dims[1]);
}

BBox3fa merge(const BBox3fa &a, const BBox3fa &b)
{
    BBox3fa out{};
    Vec3::min(out.lower, a.lower, b.lower);
    Vec3::max(out.upper, a.upper, b.upper);
    return out;
}

/* This function is called by the builder to signal progress and to
 * report memory consumption. */
bool memoryMonitor(void *userPtr, ssize_t bytes, bool post)
{
    return true;
}

bool buildProgress(void *userPtr, double f)
{
    return true;
}

void splitPrimitive(const RTCBuildPrimitive *prim, unsigned int dim, float pos, RTCBounds *lprim, RTCBounds *rprim, void *userPtr)
{
    assert(dim < 3);
    assert(prim->geomID == 0);
    *(BBox3fa *)lprim = *(BBox3fa *)prim;
    *(BBox3fa *)rprim = *(BBox3fa *)prim;
    (&lprim->upper_x)[dim] = pos;
    (&rprim->lower_x)[dim] = pos;
}

struct Node
{
    virtual float sah() = 0;
};

struct InnerNode : public Node
{
    BBox3fa bounds[2];
    Node *children[2];

    InnerNode()
    {
        bounds[0] = bounds[1] = {};
        children[0] = children[1] = nullptr;
    }

    float sah()
    {
        return 1.0f + (area(bounds[0]) * children[0]->sah() + area(bounds[1]) * children[1]->sah()) / area(merge(bounds[0], bounds[1]));
    }

    static void *create(RTCThreadLocalAllocator alloc, unsigned int numChildren, void *userPtr)
    {
        assert(numChildren == 2);
        void *ptr = rtcThreadLocalAlloc(alloc, sizeof(InnerNode), 16);
        return (void *)new (ptr) InnerNode;
    }

    static void setChildren(void *nodePtr, void **childPtr, unsigned int numChildren, void *userPtr)
    {
        assert(numChildren == 2);
        for (size_t i = 0; i < 2; i++)
            ((InnerNode *)nodePtr)->children[i] = (Node *)childPtr[i];
    }

    static void setBounds(void *nodePtr, const RTCBounds **bounds, unsigned int numChildren, void *userPtr)
    {
        assert(numChildren == 2);
        for (size_t i = 0; i < 2; i++)
            ((InnerNode *)nodePtr)->bounds[i] = *(const BBox3fa *)bounds[i];
    }
};

struct LeafNode : public Node
{
    unsigned id;
    BBox3fa bounds;

    LeafNode(unsigned id, const BBox3fa &bounds)
        : id(id), bounds(bounds) {}

    float sah()
    {
        return 1.0f;
    }

    static void *create(RTCThreadLocalAllocator alloc, const RTCBuildPrimitive *prims, size_t numPrims, void *userPtr)
    {
        assert(numPrims == 1);
        void *ptr = rtcThreadLocalAlloc(alloc, sizeof(LeafNode), 16);
        return (void *)new (ptr) LeafNode(prims->primID, *(BBox3fa *)prims);
    }
};
