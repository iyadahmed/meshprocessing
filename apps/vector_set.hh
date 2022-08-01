#pragma once

#include <cstddef>

#include "MurmurHash3.h"

/* A set data structure which only grows (cannot delete elements) and can be
 * indexed */
template <typename T> struct VectorSet {
private:
  struct Node {
    T value;
    size_t next;
  };

  Node *pool;
  size_t pool_cap;
  size_t pool_count;

  size_t *buckets;
  size_t bucket_count;

  size_t new_node() {
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
    return node_index;
  }

public:
  VectorSet(size_t bucket_count = 1024) {
    pool_cap = 1;
    pool_count = 0;
    pool = new Node[pool_cap];

    this->bucket_count = bucket_count;
    buckets = new size_t[this->bucket_count]{SIZE_MAX};
    for (size_t i = 0; i < this->bucket_count; i++) {
      buckets[i] = SIZE_MAX;
    }
  }

  ~VectorSet() {
    // FIXME: avoid double free when copying or moving object
    delete[] pool;
    delete[] buckets;
  }

  size_t insert(T value) {
    uint32_t hash;
    MurmurHash3_x86_32(&value, sizeof(T), 0, &hash);
    size_t bucket_index = hash % bucket_count;

    // Check if value already exists
    size_t node_iter = buckets[bucket_index];
    while (node_iter != SIZE_MAX) {
      if (pool[node_iter].value == value) {
        return node_iter;
      }
      node_iter = pool[node_iter].next;
    }

    // Insert new value
    size_t node_index = new_node();
    pool[node_index].value = value;
    pool[node_index].next = buckets[bucket_index];
    buckets[bucket_index] = node_index;
    return node_index;
  }

  T &operator[](size_t index) { return pool[index]; }

  size_t count() const { return pool_count; }
};