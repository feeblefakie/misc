#include <iostream>
#include <stdint.h>
#include <cstring>
#include <unistd.h>

int main(void) {
  int64_t a = 1;
  int64_t b = 500;

  for (int64_t a = 0; a < 1000; a++) {
    if (memcmp(&a, &b, sizeof(int64_t)) == 0) {
      std::cout << a << " is smaller than " << b << std::endl;
    }
  }

  return 0;
}
