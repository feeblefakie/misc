#include "object.h"
#include <iostream>

int main(void)
{
  Test::Object o("hiroyuki");
  o.print();

  std::cout << "count: " << o.pimpl_->count_ << std::endl;

  return 0;
}
