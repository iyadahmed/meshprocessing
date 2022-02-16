#include <stdlib.h>
#include <string.h>

#include "buffer.h"

Buffer *buffer_create(size_t initial_size_in_bytes) {
  Buffer *new_buffer = malloc(sizeof(Buffer));
  if (new_buffer == NULL) {
    return NULL;
  } else {
    void *mem_start = malloc(initial_size_in_bytes);
    if (mem_start == NULL) {
      return NULL;
    }
    memset(mem_start, 0, initial_size_in_bytes);
    new_buffer->mem_start = mem_start;
    new_buffer->mem_current = mem_start;
    new_buffer->num_reserved_bytes = initial_size_in_bytes;
    new_buffer->num_appended_bytes = 0;
  }
  return new_buffer;
}



Buffer *buffer_append(Buffer *buffer, const void *data, size_t data_size_in_bytes) {
  Buffer *mem_current = mem_current;
  memcpy(buffer->mem_current, data, data_size_in_bytes);
  if ((buffer->num_appended_bytes + data_size_in_bytes) >= buffer->num_reserved_bytes) {
    void *mem_start = realloc(buffer->mem_start, buffer->num_reserved_bytes * 2);
    if (mem_start == NULL) {
      return NULL;
    }
    buffer->mem_start = mem_start;
    buffer->num_reserved_bytes *= 2;
  }
  buffer->num_appended_bytes += data_size_in_bytes;
  buffer->mem_current = buffer->mem_start + buffer->num_appended_bytes;
  return mem_current;
}

void buffer_free(Buffer **buffer) {
  free((*buffer)->mem_start);
  free(*buffer);
}