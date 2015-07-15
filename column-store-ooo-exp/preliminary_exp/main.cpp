#include <iostream>
#include "column.h"

int main(void) {
  cstore::Column *c = new cstore::Column();
  c->set_column_type(cstore::NOCOMP, cstore::INT);
  c->open("col1", COL_CREAT);
  cstore::nocomp_val4_t v;
  int i = 10;
  memcpy(&(v.val), &i, sizeof(int));
  c->append(&v);
  i = 20;
  memcpy(&(v.val), &i, sizeof(int));
  c->append(&v);
  i = 30;
  memcpy(&(v.val), &i, sizeof(int));
  c->append(&v);
  delete(c);

  cstore::Column *c3 = new cstore::Column();
  c3->set_column_type(cstore::RLE, cstore::BIGINT);
  c3->open("col3", COL_CREAT);
  cstore::rle_triple_val8_t v3;
  memcpy(v3.val, "19920303", 8);
  v3.start = 0;
  v3.length = 100;
  c3->append(&v3);
  memcpy(v3.val, "19930101", 8);
  v3.start = 100;
  v3.length = 100;
  c3->append(&v3);
  memcpy(v3.val, "19940404", 8);
  v3.start = 200;
  v3.length = 100;
  c3->append(&v3);
  delete(c3);

  cstore::Column *c2 = new cstore::Column();
  c2->open("col1", COL_RDONLY);

  cstore::nocomp_val4_t rv;
  c2->lookup_bypos(0, &rv);
  std::cout << *(int *) rv.val << std::endl;
  c2->lookup_bypos(1, &rv);
  std::cout << *(int *) rv.val << std::endl;
  c2->lookup_bypos(2, &rv);
  std::cout << *(int *) rv.val << std::endl;

  int pred = 30;
  while (true) {
    //bool ret = c2->get_next(NULL, &rv);
    bool ret = c2->get_next(&pred, &rv);
    if (!ret) {
      break;
    }
    std::cout << "get_next: " << *(int *) rv.val << std::endl;
  }
  delete(c2);


  cstore::Column *c4 = new cstore::Column();
  c4->open("col3", COL_RDONLY);
  cstore::rle_triple_val8_t rlev;

  c4->lookup_bypos(1, &rlev);
  std::cout << "pos 1: ";
  std::cout.write(rlev.val, sizeof(rlev.val));
  std::cout << std::endl;

  char *pred2 = "19940404";
  while (true) {
    //bool ret = c4->get_next(NULL, &rlev);
    bool ret = c4->get_next(pred2, &rlev);
    if (!ret) {
      break;
    }
    std::cout << "get_next: ";
    std::cout.write(rlev.val, sizeof(rlev.val));
    std::cout << std::endl;
  }

  return 0;
}
