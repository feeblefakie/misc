#include <iostream>
#include <vector>
#include <boost/pool/pool_alloc.hpp>

int main(void)
{
  std::vector< int, boost::pool_allocator<int> > vec1;
  std::vector< int, boost::pool_allocator<int> > vec2;

  while (1) { 
    for (int i = 0; i < 10000000; ++i) {
      vec1.push_back(i);
    }
    vec1.clear();
    std::cout << "vec1 done" << std::endl;
    for (int i = 0; i < 1000000; ++i) {
      vec2.push_back(i);
    }
    vec2.clear();
    std::cout << "vec2 done" << std::endl;
  }

  return 0;
}
