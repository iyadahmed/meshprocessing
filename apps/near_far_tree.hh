#pragma once

#include <cstddef>

struct NearFarTree {
  struct Node {
    float x, y, z;
    Node *near, *far;
    float distance_squared(Node *other) {
      return x * other->x + y * other->y + z * other->z;
    }
  };

  Node *nodes;
  Node *root;
  size_t cap;
  size_t num;

  NearFarTree(size_t max_num_points) {
    if (max_num_points == 0) {
      throw "Number of points cannot be zero";
    }
    cap = max_num_points;
    nodes = new Node[cap];
    num = 0;
    root = nullptr;
  }
  ~NearFarTree() {
    // FIXME: avoid double free when copying/moving object
    delete[] nodes;
  }
  typedef Node *NodePtr;
  Node *new_node(float x, float y, float z) {
    Node *nn = nodes + (num++);
    nn->x = x;
    nn->y = y;
    nn->z = z;
    nn->near = nullptr;
    nn->far = nullptr;
    return nn;
  }
  void insert(NodePtr &node, float x, float y, float z) {

    if (node == nullptr) {
      node = new_node(x, y, z);
      return;
    }

    float dx = x - node->x;
    float dy = y - node->y;
    float dz = z - node->z;
    float d = dx * dx + dy * dy + dz * dz;

    if (node->near == nullptr) {
      Node *nn = new_node(x, y, z);
      node->near = nn;
      return;
    }

    float dnx = x - node->near->x;
    float dny = y - node->near->y;
    float dnz = z - node->near->z;
    float dnear = dnx * dnx + dny * dny + dnz * dnz;

    if (d < dnear) {
      Node *nn = new_node(x, y, z);
      nn->near = node->near;
      node->near = nn;
      return;
    }

    if (node->far == nullptr) {
      Node *nn = new_node(x, y, z);
      node->far = nn;
      return;
    }

    float dfx = x - node->far->x;
    float dfy = y - node->far->y;
    float dfz = z - node->far->z;
    float dfar = dfx * dfx + dfy * dfy + dfz * dfz;

    if (d > dfar) {
      Node *nn = new_node(x, y, z);
      nn->near = node->far;
      node->far = nn;
      return;
    }

    if (dfar < dnear) {
      insert(node->far, x, y, z);
    } else {
      insert(node->near, x, y, z);
    }
  }
};
