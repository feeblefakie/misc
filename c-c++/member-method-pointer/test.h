#include <iostream>

class Test {
public:
  Test(void)
  {
    std::cout << "Test constructor" << std::endl;
  }
  void func()
  {
    std::cout << "Test::func()" << std::endl;
  }
};

typedef void (Test::*MemberFunc)();
