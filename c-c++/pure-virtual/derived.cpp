#include "derived.h"
#include <iostream>

Derived::Derived(void)
{
  std::cout << "Derived constructor" << std::endl;
}
Derived::~Derived(void)
{
  std::cout << "Derived destructor" << std::endl;
}

void Derived::exec(void)
{
  std::cout << "Derived::exec" << std::endl;
  std::cout << "testbase: " << testbase << std::endl;
}
