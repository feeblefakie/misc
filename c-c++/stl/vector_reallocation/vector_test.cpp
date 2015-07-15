#include <vector>
#include <list>
#include <set>
#include <iostream>

template <class T>    
struct less_term : public std::binary_function <T, T, bool> {
  bool operator() (const T& t1, const T& t2) const {
    return t1.key < t2.key;
  }   
};  

typedef std::vector<int> plist;
typedef std::list<int> plist2;

struct Buffer {
  Buffer(std::string key_)
  : key(key_)
  {}  
  std::string key;
  plist val;
};  

typedef std::set< Buffer, less_term<Buffer> > BufferSet;

int main(void)
{
  std::vector<int> ary;
  BufferSet bs;

  for (int j = 0; j < 1000000; ++j) {

    //Buffer buffer("hello");
    //buffer.val = ary;
    plist val;
    //bs.insert(buffer);
    //BufferSet::iterator itr = bs.begin();
    for (int i = 0; i < 50000000; ++i) {
      //const_cast< plist & >(itr->val).push_back(i);
      val.push_back(i);
    }
    std::cout << "pushed " << j << std::endl;
    //const_cast< plist & >(itr->val).clear();
    //bs.clear();

    val.clear();
    std::cout << "cleared " << j << std::endl;

  }

  return 0;
}

