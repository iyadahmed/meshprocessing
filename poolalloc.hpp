/* Pool allocator from http://dmitrysoshnikov.com/compilers/writing-a-pool-allocator/ */

#ifndef POOLALLOC_HPP
#define POOLALLOC_HPP

#include <cstddef>

struct Chunk {
  /**
   * When a chunk is free, the `next` contains the
   * address of the next chunk in a list.
   *
   * When it's allocated, this space is used by
   * the user.
   */
  Chunk *next;
};

/**
 * The allocator class.
 *
 * Features:
 *
 *   - Parametrized by number of chunks per block
 *   - Keeps track of the allocation pointer
 *   - Bump-allocates chunks
 *   - Requests a new larger block when needed
 *
 */
class PoolAllocator {
public:
  PoolAllocator(size_t chunks_per_block) : m_chunks_per_block(chunks_per_block) {}

  void *allocate(size_t size);
  void deallocate(void *ptr, size_t size);

private:
  size_t m_chunks_per_block;
  Chunk *m_allocation_pointer = nullptr;

  /**
   * Allocates a larger block (pool) for chunks.
   */
  Chunk *allocate_block(size_t chunkSize);
};

#endif