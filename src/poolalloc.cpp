/* Pool allocator from http://dmitrysoshnikov.com/compilers/writing-a-pool-allocator/ */

#include <cstddef>
#include <iostream>

#include "poolalloc.hpp"

PoolAllocator::~PoolAllocator() {
  for (int i = 0; i < m_num_blocks; i++) {
    free(m_blocks[i]);
  }
}

/**
 * Allocates a new block from OS.
 *
 * Returns a Chunk pointer set to the beginning of the block.
 */
Chunk *PoolAllocator::allocate_block(size_t chunk_size) {
  size_t block_size = m_chunks_per_block * chunk_size;

  void *new_block_mem = malloc(block_size);
  if (new_block_mem == NULL) {
    return NULL;
  }

  Chunk *block_begin = reinterpret_cast<Chunk *>(new_block_mem);
  Chunk *chunk = block_begin;

  for (int i = 0; i < m_chunks_per_block - 1; ++i) {
    chunk->next = reinterpret_cast<Chunk *>(reinterpret_cast<char *>(chunk) + chunk_size);
    chunk = chunk->next;
  }

  chunk->next = nullptr;

  m_num_blocks++;

  void *m_blocks_mem;
  if (m_blocks == nullptr) {
    m_blocks_mem = malloc(sizeof(Chunk *));
  } else {
    m_blocks_mem = realloc(m_blocks, ++m_num_blocks);
  }

  if (m_blocks_mem == NULL) {
    free(new_block_mem);
    return NULL;
  }

  m_blocks[m_num_blocks - 1] = block_begin;

  return block_begin;
}

/**
 * Returns the first free chunk in the block.
 *
 * If there are no chunks left in the block,
 * allocates a new block.
 */
void *PoolAllocator::allocate(size_t size) {
  if (m_allocation_pointer == nullptr) {
    m_allocation_pointer = allocate_block(size);
  }
  Chunk *free_chunk = m_allocation_pointer;
  m_allocation_pointer = m_allocation_pointer->next;

  return free_chunk;
}

/**
 * Puts the chunk into the front of the chunks list.
 */
void PoolAllocator::deallocate(void *chunk, size_t size) {

  reinterpret_cast<Chunk *>(chunk)->next = m_allocation_pointer;
  m_allocation_pointer = reinterpret_cast<Chunk *>(chunk);
}