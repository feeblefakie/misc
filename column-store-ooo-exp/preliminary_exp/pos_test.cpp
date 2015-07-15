#include <iostream>
#include "column.h"

int main(int argc, char *argv[]) {

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " pos" << std::endl;
    exit(1);
  }
  off_t pos = atoll(argv[1]);
  std::cout << "looking " << pos << std::endl;

  cstore::Column *c1 = new cstore::Column();
  c1->open("shipdate", COL_RDONLY);

  cstore::Column *c2 = new cstore::Column();
  c2->open("linenumber", COL_RDONLY);

  cstore::rle_triple_val8_t rv1;
  c1->lookup_bypos(pos, &rv1);
  std::cout.write(rv1.val, 8);
  std::cout << std::endl;
  std::cout << rv1.start << ", " << rv1.length << std::endl;

  /*
  cstore::rle_triple_val1_t rv2;
  c2->lookup_bypos(pos, &rv2);
  int val = 0;
  memcpy(&val, &rv2.val, 1);
  std::cout << val << std::endl;
  std::cout << rv1.start << ", " << rv1.length << std::endl;
  */
  cstore::nocomp_val1_t rv2;
  c2->lookup_bypos(pos, &rv2);
  int val = 0;
  memcpy(&val, &rv2.val, 1);
  std::cout << val << std::endl;

  delete(c1);
  delete(c2);

  return 0;
}
