#include <iostream>
#include <vector>
#include <boost/pool/pool_alloc.hpp>

int main(void)
{
  std::vector< int, boost::pool_allocator<int> > vec1;
  std::vector< int, boost::pool_allocator<int> > vec2;


  vec1.push_back(10);

  vec2 = vec1;

  return 0;
}
