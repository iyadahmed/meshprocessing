#pragma once

#include <cstddef>

#include "MurmurHash3.h"

/* A set data structure which only grows (cannot delete elements) and can be
 * indexed */
template <typename T> struct VectorSet {
  struct Node {
    T value;
    size_t index;
    Node *next;
  };

  Node *pool;
  size_t pool_cap;
  size_t pool_count;

  Node **buckets;
  size_t bucket_count;

  VectorSet(size_t bucket_count = 1024) {
    pool_cap = 1;
    pool_count = 0;
    pool = new Node[pool_cap];

    this->bucket_count = bucket_count;
    buckets = new Node *[this->bucket_count] { nullptr };
  }

  ~VectorSet() {
    // FIXME: avoid double free when copying or moving object
    delete[] pool;
    delete[] buckets;
  }

  Node *new_node() {
    if (pool_count == pool_cap) {
      size_t new_cap = pool_cap * 2;
      Node *new_pool = new Node[new_cap];
      for (size_t i = 0; i < pool_count; i++) {
        new_pool[i] = pool[i];
      }
      delete[] pool;
      pool = new_pool;
      pool_cap = new_cap;
    }

    size_t node_index = (pool_count++);
    Node *node = pool + node_index;
    node->index = node_index;
    return node;
  }

  size_t insert(T value) {
    uint32_t hash;
    MurmurHash3_x86_32(&value, sizeof(T), 0, &hash);
    size_t bucket_index = hash % bucket_count;

    Node *node_iter = buckets[bucket_index];
    while (node_iter != nullptr) {
      if (node_iter->value == value) {
        return node_iter->index;
      }
      node_iter = node_iter->next;
    }
    Node* node = new_node();
    node->value = value;
    node->next = buckets[bucket_index];
    buckets[bucket_index] = node;
    return node->index;
  }
};