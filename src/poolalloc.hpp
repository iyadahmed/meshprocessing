#ifndef POOLALLOC_HPP
#define POOLALLOC_HPP

#include <cstddef>

struct Chunk {
  Chunk *next;
};

class PoolAllocator {
public:
  void *allocate(size_t size);
  void deallocate(void *ptr, size_t size);

private:
  size_t m_chunks_per_block = 1024UL;
  Chunk *m_allocation_pointer = nullptr;
  Chunk **m_blocks = nullptr;
  int m_num_blocks = 0;

  Chunk *allocate_block(size_t chunk_size);
};

#endif