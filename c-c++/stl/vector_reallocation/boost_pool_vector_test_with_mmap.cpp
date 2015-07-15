#include <vector>
#include <list>
#include <set>
#include <iostream>
#include <boost/pool/pool_alloc.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

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

  int fd = open("./test.map", O_CREAT | O_RDWR | O_TRUNC, 00644);
  if (fd < 0) {
    perror("ftruncate failed");
    exit(1);
  }

  size_t size = 536870913;
  if (ftruncate(fd, size) < 0) {
    fprintf(stderr, "ftruncate failed.\n");
    exit(1);
  }
  void *map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  
  if (map == MAP_FAILED) {
    perror("mmap failed");
  }
  printf("mmaped\n");

  for (int j = 0; j < 10; ++j) {
    /* unmap */
    if (munmap(map, size) < 0) {
      perror("munmap failed");
      break;
    }
    printf("munmaped\n");

    //Buffer buffer("hello");
    //buffer.val = ary;
    plist val;
    //std::vector<int, boost::pool_allocator<int> > val;
   
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

    /* map again */
    map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  
    if (map == MAP_FAILED) {
      perror("mmap failed");
      break;
    }
    printf("mmaped");
  }

  return 0;
}
