#include <string>
#include <iostream>

void func(const std::string &str);

int main(void)
{
  std::string a = "aaa";
  func(a);
  func(std::string("bbb"));

  return 0;
}

void func(const std::string &str)
{
  std::cout << str << std::endl;
}
