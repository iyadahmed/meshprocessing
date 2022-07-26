#pragma once

struct BSPNode {
  float x = 0.0f, y = 0.0f, z = 0.0f;
  float nx = 0.0f, ny = 0.0f,
        nz = 0.0f; // Plane normal (no need to be normalized)
  BSPNode *left = nullptr, *right = nullptr;
  bool is_leaf() const { return (left == nullptr) && (right == nullptr); }
};

struct BSPTree {
  BSPNode *nodes;
  size_t cap;
  size_t num;

  BSPTree(size_t max_num_points) {
    if (max_num_points == 0)
      throw;
    cap = 2 * max_num_points - 1;
    nodes = new BSPNode[cap];
    num = 0;
  }

  ~BSPTree()
  {
	// FIXME: avoid double free when copying object
	delete [] nodes;
  }

  BSPNode *new_node() {
    if (num > cap)
      throw;
    return nodes + (num++);
  }

  void insert(float x, float y, float z) {
    if (num == 0) {
      auto node = this->new_node();
      node->x = x;
      node->y = y;
      node->z = z;
      node->nx = 0.0f;
      node->ny = 0.0f;
      node->nz = 0.0f;
      node->left = node->right = nullptr;
    } else {
      BSPNode *node = nodes;
      // Find empty leaf to insert new node
      while (node->left && node->right) {
        float dx = x - node->x;
        float dy = y - node->y;
        float dz = z - node->z;
        float d = dx * node->nx + dy * node->ny + dz * node->nz;
        if (d > 0.0f) {
          node = node->left;
        } else {
          node = node->right;
        }
      }
      BSPNode *new_node = this->new_node();
      new_node->x = x;
      new_node->y = y;
      new_node->z = z;
      new_node->nx = node->x - x;
      new_node->ny = node->y - y;
      new_node->nz = node->z - z;
      new_node->left = node->right = nullptr;
      float dx = x - node->x;
      float dy = y - node->y;
      float dz = z - node->z;
      float d = dx * node->nx + dy * node->ny + dz * node->nz;
      if (d > 0.0f) {
        node->left = new_node;
      } else {
        node->right = new_node;
      }
    }
  }
};
