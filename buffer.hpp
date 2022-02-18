#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstdint>
#include <cstdlib>
#include <cstring>

template <typename T, uint32_t initial_cap> class Buffer {
public:
  T *mem_start;
  uint32_t cap;
  uint32_t count;

  Buffer() {
    this->mem_start = new T[initial_cap]();
    this->cap = initial_cap;
    this->count = 0;
  }

  ~Buffer() { delete[] this->mem_start; }

  T &get_mem() {
    if (this->count >= this->cap) {
      auto new_cap = this->count * 2;
      auto new_mem = new T[new_cap]();
      // Update memory
      // TODO: copy old memory to new memory
      delete[] this->mem_start;
      this->mem_start = new_mem;
      // Update cap
      this->cap = new_cap;
    }
    return this->mem_start[this->count++];
  }

  T &operator[](const int index) { return this->mem_start[index]; }
};

#endif