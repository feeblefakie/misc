#include "class.h"
#include <iostream>

MyClass::MyClass(void)
{
  std::cout << "MyClass constructor" << std::endl;
}

MyClass::~MyClass(void)
{
  std::cout << "MyClass destructor" << std::endl;
}
