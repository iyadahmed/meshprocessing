#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstdint>
#include <cstdlib>
#include <cstring>

template <class T> class Buffer {
public:
  T *mem_start;
  uint32_t num_reserved_items;
  uint32_t num_appended_items;

  Buffer(uint32_t initial_num_items) {
    this->mem_start = (T *)std::malloc(initial_num_items * sizeof(T));
    this->num_appended_items = 0;
    this->num_reserved_items = initial_num_items;
  }

  ~Buffer() { std::free(this->mem_start); }

  void append(T *data_ptr) {
    if (this->num_appended_items == this->num_reserved_items) {
      this->mem_start = (T *)realloc(this->mem_start, this->num_reserved_items * 2 * sizeof(T));
    }
    memcpy(this->mem_start + this->num_appended_items, data_ptr, sizeof(T));
    this->num_appended_items++;
  }

  T &operator[](const int index) { return &(this->mem_start[index]); }
};

#endif