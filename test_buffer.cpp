#include <iostream>

#include "buffer.hpp"

int main(void) {
  auto buf = new Buffer<int>(100U);

  for (int i = 0; i < 100; i++) {
    buf->append(&i);
  }

  std::cout << buf->mem_start[10] << std::endl;
  delete buf;
  return 0;
}