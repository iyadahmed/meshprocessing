#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

typedef struct Buffer {
  void *mem_start;
  void *mem_current;
  size_t num_reserved_bytes;
  size_t num_appended_bytes;
} Buffer;

Buffer *buffer_create(size_t initial_size_in_bytes);
Buffer *buffer_append(Buffer *buffer, const void *data, size_t data_size_in_bytes);
void buffer_free(Buffer **buffer);

#endif