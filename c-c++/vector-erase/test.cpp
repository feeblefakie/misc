#include <iostream>
#include <vector>

int main(void)
{
  std::vector<int> vec;
  for (int i = 0; i < 10; ++i) {
    vec.push_back(i);
  }

  std::vector<int>::iterator itr = vec.begin();
  std::vector<int>::iterator itr_end = vec.end();
  for (int i = 0; i != 3 && itr != itr_end; ++i) {
    vec.erase(itr);
    itr = vec.begin();
  }

  std::cout << "#####" << std::endl;
  for (int i = 0; i < vec.size(); ++i) {
    std::cout << vec[i] << std::endl;
  }
  itr = vec.begin();
  itr_end = vec.end();
  std::cout << "#####" << std::endl;
  for (; itr != itr_end; ++itr) {
    std::cout << *itr << std::endl;
  }

  return 0;
}
