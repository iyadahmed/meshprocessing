#pragma once

#include <unordered_set>
#include <utility> // for std::swap
#include <vector>

struct IndexedTriangle
{
    int v1, v2, v3;
    IndexedTriangle(int v1_, int v2_, int v3_)
    {
        v1 = v1_;
        v2 = v2_;
        v3 = v3_;
        // https://stackoverflow.com/a/22901187/8094047
        if (v1 > v2)
            std::swap(v1, v2);
        if (v1 > v3)
            std::swap(v1, v3);
        if (v2 > v3)
            std::swap(v2, v3);
    }
};

bool operator==(const IndexedTriangle &lhs, const IndexedTriangle &rhs)
{
    return (lhs.v1 == rhs.v1) && (lhs.v2 == rhs.v2) && (lhs.v3 == rhs.v3);
}

template <>
struct std::hash<IndexedTriangle>
{
    std::size_t operator()(IndexedTriangle const &value) const noexcept
    {
        // https://stackoverflow.com/a/16794345/8094047
        size_t h1 = std::hash<int>()(value.v1);
        size_t h2 = std::hash<int>()(value.v2);
        size_t h3 = std::hash<int>()(value.v3);
        return (h1 ^ (h2 << 1)) ^ h3;
    }
};

struct IndexedEdge
{
    int v1, v2;
    IndexedEdge(int v1_, int v2_)
    {
        v1 = v1_;
        v2 = v2_;
        if (v1 > v2)
        {
            std::swap(v1, v2);
        }
    }
};

bool operator==(const IndexedEdge &lhs, const IndexedEdge &rhs)
{
    return (lhs.v1 == rhs.v1) && (lhs.v2 == rhs.v2);
}

template <>
struct std::hash<IndexedEdge>
{
    std::size_t operator()(IndexedEdge const &value) const noexcept
    {
        // https://stackoverflow.com/a/16794345/8094047
        size_t h1 = std::hash<int>()(value.v1);
        size_t h2 = std::hash<int>()(value.v2);
        return h1 ^ (h2 << 1);
    }
};

struct IndexedVertex
{
    float x, y, z;
};

bool operator==(const IndexedVertex &lhs, const IndexedVertex &rhs)
{
    return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
}

template <>
struct std::hash<IndexedVertex>
{
    std::size_t operator()(IndexedVertex const &value) const noexcept
    {
        // https://stackoverflow.com/a/16794345/8094047
        size_t h1 = std::hash<float>()(value.x);
        size_t h2 = std::hash<float>()(value.y);
        size_t h3 = std::hash<float>()(value.z);
        return (h1 ^ (h2 << 1)) ^ h3;
    }
};

struct IndexedMesh
{
    std::unordered_set<IndexedVertex> verts_;
    std::vector<IndexedVertex> verts;
    std::vector<IndexedEdge> edges;
    std::vector<IndexedTriangle> tris;
};