#include <iostream>
#include "ops.h"

int main(int argc, char *argv[]) {

  cstore::Column *c1 = new cstore::Column();
  c1->open("shipdate", COL_RDONLY);
  //c1->open("linenumber", COL_RDONLY);

  //uint8_t pred = 2;
  cstore::DS2 *ds2 = new cstore::DS2(c1, (void *) "19920110", cstore::LT);
  //cstore::DS2 *ds2 = new cstore::DS2(c1, (void *) &pred, cstore::LT);
  off_t pos;
  char *val;
  while (true) {
    bool ret = ds2->rle_op(&pos, (void **) &val);
    //bool ret = ds2->nocomp_op(&pos, (void **) &val);
    if (!ret) {
      break;
    }
    std::cout << "pos: " << pos << std::endl;
    std::cout << "val: ";
    std::cout.write(val, 8);
    //std::cout << (int) val[0];
    std::cout << std::endl;
  }

  delete(c1);

  return 0;
}
