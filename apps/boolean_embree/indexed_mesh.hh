#pragma once

#include <unordered_map>
#include <utility> // for std::swap
#include <vector>

#include "vec3.hh"

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

template <>
struct std::hash<Vec3>
{
    std::size_t operator()(Vec3 const &value) const noexcept
    {
        // https://stackoverflow.com/a/16794345/8094047
        size_t h1 = std::hash<float>()(value.x);
        size_t h2 = std::hash<float>()(value.y);
        size_t h3 = std::hash<float>()(value.z);
        return (h1 ^ (h2 << 1)) ^ h3;
    }
};

class IndexedMesh
{
public:
    std::unordered_map<Vec3, size_t> vert_indices_map;
    std::vector<Vec3> verts;

    std::unordered_map<IndexedEdge, size_t> edge_indices_map;
    std::vector<IndexedEdge> edges;

    std::unordered_map<IndexedTriangle, size_t> tris_indices_map;
    std::vector<IndexedTriangle> tris;

    int add_vertex(Vec3 pos)
    {
        auto it = vert_indices_map.find(pos);
        if (it != vert_indices_map.end())
        {
            return it->second;
        }
        size_t new_index = verts.size();
        vert_indices_map[pos] = new_index;
        verts.push_back(pos);
        return new_index;
    };

    int add_edge(int v1, int v2)
    {
        auto it = edge_indices_map.find({v1, v2});
        if (it != edge_indices_map.end())
        {
            return it->second;
        }
        size_t new_index = edges.size();
        edge_indices_map[{v1, v2}] = new_index;
        edges.push_back({v1, v2});
        return new_index;
    }

    int add_edge(Vec3 a, Vec3 b)
    {
        int v1 = add_vertex(a);
        int v2 = add_vertex(b);
        add_edge(v1, v2);
    }

    int add_triangle(int v1, int v2, int v3)
    {
        add_edge(v1, v2);
        add_edge(v2, v3);
        add_edge(v3, v1);

        auto it = tris_indices_map.find({v1, v2, v3});
        if (it != tris_indices_map.end())
        {
            return it->second;
        }
        size_t new_index = tris.size();
        tris_indices_map[{v1, v2, v3}] = new_index;
        tris.push_back({v1, v2, v3});
        return new_index;
    }

    int add_triangle(Vec3 a, Vec3 b, Vec3 c)
    {
        int v1 = add_vertex(a);
        int v2 = add_vertex(b);
        int v3 = add_vertex(c);
        return add_triangle(v1, v2, v3);
    }
};