#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "buffer.hpp"

template <class T> Buffer<T>::Buffer(uint32_t initial_num_items) {
  this->mem_start = std::malloc(initial_num_items * sizeof(T));
  this->num_appended_items = 0;
  this->num_reserved_items = initial_num_items;
}

template <class T> void Buffer<T>::append(T *data_ptr) {
  if (this->num_appended_items == this->num_reserved_items) {
    this->mem_start = realloc(this->mem_start, this->num_reserved_items * 2 * sizeof(T));
  }
  memcpy(this->mem_start + this->num_appended_items, data_ptr, sizeof(T));
  this->num_appended_items++;
}

template <class T> Buffer<T>::~Buffer() { std::free(this->mem_start); }

template <class T> T &Buffer<T>::operator[](const int index) { return &(this->mem_start[index]); }
