#ifndef POOLMEM_HPP
#define POOLMEM_HPP

#define POOLMEM_NODES_PER_BLOCK_DEFAULT 10240U

template <typename T> struct PoolMemNode {
  T data;
  struct PoolMemNode *next = nullptr;
};
template <typename T> struct PoolMem {
private:
  std::vector<PoolMemNode<T> *> blocks;
  PoolMemNode<T> *allocation_pointer = nullptr;

  inline PoolMemNode<T> *allocate_block() {
    auto block_begin = new PoolMemNode<T>[POOLMEM_NODES_PER_BLOCK_DEFAULT]();
    blocks.push_back(block_begin);
    auto it = block_begin;
    for (int i = 0; i < POOLMEM_NODES_PER_BLOCK_DEFAULT - 1; i++) {
      it->next = (it + 1);
      it = it->next;
    }
    it->next = nullptr;
    return block_begin;
  }

public:
  ~PoolMem() {
    for (auto mem : blocks) {
      delete[] mem;
    }
  }

  inline PoolMemNode<T> *allocate() {
    if (allocation_pointer == nullptr) {
      allocation_pointer = allocate_block();
    }

    auto free_node = allocation_pointer;
    allocation_pointer = allocation_pointer->next;
    return free_node;
  }

  inline void deallocate(PoolMemNode<T> *node) {
    if (node != nullptr) {
      node->next = allocation_pointer;
      allocation_pointer = node;
    }
  }
};

#endif