#include <iostream>

#include "buffer.hpp"

int main(void) {
  Buffer<int> buf(1000U);

  for (int i = 0; i < 1000'000'0; i++) {
    auto mem = buf.allocate();
    if (mem){
      *mem = i;
    }
  }

  std::cout << buf.mem_start[100] << std::endl;
  return 0;
}