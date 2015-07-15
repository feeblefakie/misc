#include <iostream>

bool allocate(void);

int main(void)
{
  for (int i = 0; i < 1000000; ++i) {
    if (!allocate()) {
      std::cerr << "allocate failed" << std::endl;
      exit(1);
    }
  }

  return 0;
}

bool allocate(void)
{
  char *p = NULL;
  try {
    p = new char[1024];
  } catch (std::bad_alloc &e) {
    std::cerr << e.what() << std::endl;
    delete [] p;
    return false;
  }

  memset(p, 0, 1024);
  strncpy(p, "hello", 1024);

  delete [] p;

  return true;
} 
