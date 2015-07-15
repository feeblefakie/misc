#include <stdlib.h>
#include <iostream>

int main(int argc, char *argv[])  {

  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " num cardinality repeat" << std::endl;
    exit(1);
  }
  off_t num = atoll(argv[1]);
  int card = atoi(argv[2]);
  int repeat = atoi(argv[3]);

  srand((unsigned) time(NULL));
  off_t i = 0;
  while (true) {
    int val = rand() % card;
    for (int j = 0; j < repeat; ++j) {
      std::cout << val << std::endl;
    }
    i += repeat;
    if (i >= num) {
      break;
    }
  }

  return 0;
}
