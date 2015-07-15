#include "test.h"
#include <iostream>

int main(void)
{
  char *buf = Test::process();

  std::cout << buf << std::endl;

  return 0;
}
