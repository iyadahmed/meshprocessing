/* Pool allocator from http://dmitrysoshnikov.com/compilers/writing-a-pool-allocator/ */

#include <cstddef>
#include <iostream>

#include "poolalloc.hpp"

/**
 * Allocates a new block from OS.
 *
 * Returns a Chunk pointer set to the beginning of the block.
 */
Chunk *PoolAllocator::allocate_block(size_t chunk_size) {
  std::cout << "\nAllocating block (" << m_chunks_per_block << " chunks):\n\n";

  size_t blockSize = m_chunks_per_block * chunk_size;

  // The first chunk of the new block.
  Chunk *block_begin = reinterpret_cast<Chunk *>(malloc(blockSize));

  // Once the block is allocated, we need to chain all
  // the chunks in this block:

  Chunk *chunk = block_begin;

  for (int i = 0; i < m_chunks_per_block - 1; ++i) {
    chunk->next = reinterpret_cast<Chunk *>(reinterpret_cast<char *>(chunk) + chunk_size);
    chunk = chunk->next;
  }

  chunk->next = nullptr;

  return block_begin;
}

/**
 * Returns the first free chunk in the block.
 *
 * If there are no chunks left in the block,
 * allocates a new block.
 */
void *PoolAllocator::allocate(size_t size) {

  // No chunks left in the current block, or no any block
  // exists yet. Allocate a new one, passing the chunk size:

  if (m_allocation_pointer == nullptr) {
    m_allocation_pointer = allocate_block(size);
  }

  // The return value is the current position of
  // the allocation pointer:

  Chunk *free_chunk = m_allocation_pointer;

  // Advance (bump) the allocation pointer to the next chunk.
  //
  // When no chunks left, the `mAlloc` will be set to `nullptr`, and
  // this will cause allocation of a new block on the next request:

  m_allocation_pointer = m_allocation_pointer->next;

  return free_chunk;
}

/**
 * Puts the chunk into the front of the chunks list.
 */
void PoolAllocator::deallocate(void *chunk, size_t size) {

  // The freed chunk's next pointer points to the
  // current allocation pointer:

  reinterpret_cast<Chunk *>(chunk)->next = m_allocation_pointer;

  // And the allocation pointer is now set
  // to the returned (free) chunk:

  m_allocation_pointer = reinterpret_cast<Chunk *>(chunk);
}