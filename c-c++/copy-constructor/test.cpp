#include "Test.h"
#include <vector>
#include <iostream>

void get_test(std::vector<Test> &arr);

int main(void)
{
  std::vector<Test> tarray;
  get_test(tarray);

  Test t2 = tarray.at(0);

  std::cout << "t2 score: " << t2.score << std::endl;
  std::cout << "t2 attr: " << *(int *) t2.attr << std::endl;

  return 0;
}

void get_test(std::vector<Test> &arr)
{
  Test t(10);
  std::cout << "t score: " << t.score << std::endl;
  std::cout << "t attr: " << *(int *) t.attr << std::endl;

  arr.push_back(t);
}
