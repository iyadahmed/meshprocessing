#pragma once

#include <cstddef>

#include "common.hh"

struct Node {
  float x, y, z;
  Node *near, *far;
};

struct NearestPoint {
  Node *nodes;
  size_t cap;
  size_t num;

  NearestPoint(size_t num_points) {
    if (num_points == 0)
      throw "NearestPoint: Number of points cannot be zero";
    cap = num_points;
    nodes = new Node[cap];
    num = 0;
  }

  ~NearestPoint() {
    // FIXME: avoid double free when copying object
    delete[] nodes;
  }

  Node *new_node_initialized() {
    tassert(num <= cap);
    Node *node = nodes + num;
    num++;
    node->x = node->y = node->z = 0.0f;
    node->near = node->far = nullptr;
    return node;
  }

  void insert(float x, float y, float z, float merge_distance = 0.0001f) {
    if (num == 0) {
      Node *node = this->new_node_initialized();
      node->x = x;
      node->y = y;
      node->z = z;
      node->near = node->far = nullptr;
      return;
    }

    Node *node = nodes;
    while (true)
    {
      // Fill near first
      if (node->near == nullptr)
      {
        node->near = this->new_node_initialized();
        node->near->x = x;
        node->near->y = y;
        node->near->z = z;
        // TODO: return index
        return;
      }
      float dx = x - node->x;
      float dy = y - node->y;
      float dz = z - node->z;
      float distance_to_parent = dx*dx + dy*dy + dz*dz;
      if (distance_to_parent < (merge_distance*merge_distance))
      {
        // TODO: return index
        return;
      }
      // TODO: handle case if point is nearest to parent than near child
      // and if further than far child, and if far child is null
    }
  }
};