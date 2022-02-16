#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstdint>

template <class T> class Buffer {
public:
  T *mem_start;
  size_t num_reserved_items;
  size_t num_appended_items;

  Buffer(uint32_t initial_num_items);
  ~Buffer();
  void append(T *data_ptr);

  T &operator[](const int index);
};

#endif