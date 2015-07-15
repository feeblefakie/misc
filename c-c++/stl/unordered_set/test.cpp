#include<iostream>
#include<set>
#include<tr1/unordered_set>
#include<tr1/functional>

#include<boost/pool/pool_alloc.hpp>
#include<boost/timer.hpp>

int main(int argc, char *argv[])
{
  const int n = 10 * 1000 * 1000;
  srand(0);

  for (size_t repeat = 0; repeat < 1; ++repeat) {
    boost::timer t;
    double lap1 = 0.0;
    double lap2;
    {
      //std::set<int> s;
      //std::set<int, std::less<int>, boost::fast_pool_allocator<int>> s;
      //std::tr1::unordered_set<int> s;
      std::tr1::unordered_set<int, std::tr1::hash<int>, std::equal_to<int>, boost::fast_pool_allocator<int> > s;

      for (int i = 0; i < n; ++i) {
        s.insert(rand() % n);
      }
      lap2 = t.elapsed();
    }
    double lap3 = t.elapsed();

    std::cout << "ctor&search " << (lap2 - lap1) << ", dtor " << (lap3 - lap2) << std::endl;
    std::cout << "sleeping. check memory size" << std::endl;
    sleep(100);
  }

return 0;
}
