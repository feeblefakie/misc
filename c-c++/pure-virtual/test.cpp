#include "test.h"
#include <iostream>

TestBase::TestBase(void) 
: testbase(10)
{
  std::cout << "TestBase constructor" << std::endl;
}

TestBase::~TestBase(void)
{
  std::cout << "TestBase destructor" << std::endl;
}

void TestBase::process(void)
{
  std::cout << "TestBase process" << std::endl;
  exec();
}

void TestBase::exec(void)
{
  std::cout << "TestBase exec" << testbase << std::endl;
}

void TestBase::method(void)
{
  std::cout << "TestBase method: testbase: " << testbase << std::endl;
}
