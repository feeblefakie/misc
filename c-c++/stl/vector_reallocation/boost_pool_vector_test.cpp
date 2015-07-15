#include <vector>
#include <list>
#include <set>
#include <iostream>
#include <boost/pool/pool_alloc.hpp>

template <class T>    
struct less_term : public std::binary_function <T, T, bool> {
  bool operator() (const T& t1, const T& t2) const {
    return t1.key < t2.key;
  }   
};  

//typedef std::vector<int> plist;
typedef std::vector<int, boost::pool_allocator<int> > plist;

struct Buffer {
  plist val;
};  

typedef std::set< Buffer, less_term<Buffer> > BufferSet;

int main(void)
{
  std::vector<uint32_t> ary;
  BufferSet bs;

  for (int j = 0; j < 10; ++j) {

    //Buffer buffer("hello");
    //buffer.val = ary;
    Buffer b;
   
    //bs.insert(buffer);
    //BufferSet::iterator itr = bs.begin();
    for (unsigned int i = 0; i < 50000000; ++i) {
      //const_cast< plist & >(itr->val).push_back(i);
      b.val.push_back(i);
    }
    std::cout << "pushed " << j << std::endl;
    //const_cast< plist & >(itr->val).clear();
    //bs.clear();

    std::cout << "cleared " << j << std::endl;
  }

  return 0;
}
