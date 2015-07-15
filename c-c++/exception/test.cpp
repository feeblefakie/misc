#include <iostream>

int main(void)
{
  //try {
    int *p = new int[2147483640];
    if (p == NULL) {
      std::cerr << "can't allocate memory" << std::endl;
    }
    /*
  } catch (std::bad_alloc &e) {
    std::cerr << "error: " << e.what() << std::endl;
  }
  */
  return 0; 
}
