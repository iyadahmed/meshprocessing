#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstdint>
#include <cstdlib>

template <typename T> class Buffer {
public:
  T *mem_start;
  uint32_t cap;
  uint32_t count;

  Buffer(uint32_t initial_cap) {
    this->mem_start = (T *)malloc(sizeof(T) * initial_cap);
    this->cap = initial_cap;
    this->count = 0U;
  }

  ~Buffer() { free(this->mem_start); }

  T *allocate() {
    if (this->count >= this->cap) {
      this->cap = this->count * 2;
      this->mem_start = (T *)realloc(this->mem_start, sizeof(T) * this->cap);
    }
    if (this->mem_start == NULL){
      return NULL;
    }
    return this->mem_start + (this->count++);
  }
};

#endif