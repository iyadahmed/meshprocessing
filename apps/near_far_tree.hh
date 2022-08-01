#pragma once

#include <cstddef>

struct NearFarTree {
  struct Node {
    float x, y, z;
    Node *near, *far;
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
  Node *new_node() { return nodes + (num++); }
  void insert(NodePtr &node, float x, float y, float z) {
    if (node == nullptr) {
      node = new_node();
      node->x = x;
      node->y = y;
      node->z = z;
      node->far = nullptr;
      node->near = nullptr;
      return;
    }

    float dx = x - node->x;
    float dy = y - node->y;
    float dz = z - node->z;
    float d = dx * dx + dy * dy + dz * dz;

    if (node->near == nullptr) {
      Node *nn = new_node();
      nn->x = x;
      nn->y = y;
      nn->z = z;
      nn->far = nullptr;
      nn->near = nullptr;
      node->near = nn;
      return;
    }

    float dnx = x - node->near->x;
    float dny = y - node->near->y;
    float dnz = z - node->near->z;
    float dnear = dnx * dnx + dny * dny + dnz * dnz;

    if (d < dnear) {
      Node *nn = new_node();
      nn->x = x;
      nn->y = y;
      nn->z = z;
      nn->near = node->near;
      nn->far = nullptr;
      node->near = nn;
      return;
    }

    if (node->far == nullptr) {
      Node *nn = new_node();
      nn->x = x;
      nn->y = y;
      nn->z = z;
      nn->far = nullptr;
      nn->near = nullptr;
      node->far = nn;
      return;
    }

    float dfx = x - node->far->x;
    float dfy = y - node->far->y;
    float dfz = z - node->far->z;
    float dfar = dfx * dfx + dfy * dfy + dfz * dfz;

    if (d > dfar) {
      Node *nn = new_node();
      nn->x = x;
      nn->y = y;
      nn->z = z;
      nn->near = node->far;
      nn->far = nullptr;
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
