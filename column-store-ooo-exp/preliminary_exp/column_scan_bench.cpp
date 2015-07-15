#include <iostream>
#include "ops.h"
#include "query_util.h"

int main(int argc, char *argv[]) {

  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " col_name col_pred col_compare" << std::endl;
    exit(1);
  }
  char *col_name = argv[1];
  char *col_pred = argv[2];
  cstore::compare_t  col_compare_type = cstore::get_compare_type(argv[3]);

  cstore::Column *c = new cstore::Column();
  if (!c->open(col_name, COL_RDONLY)) {
    std::cerr << "open failed." << std::endl;
    exit(1);
  }

  void *pred = NULL;
  if (strcmp(col_pred, "NULL") != 0) {
    pred = cstore::get_pred(col_pred, c->get_col_type());
  }
  cstore::DS2 *ds2 = new cstore::DS2(c, pred, col_compare_type);

  bool ret;
  char *val1;
  off_t pos;
  off_t n_pos_lookups = 0;
  off_t n_tuples = 0;

  while (true) {
    ret = ds2->nocomp_op(&pos, (void **) &val1);
    if (!ret) { break; }
    n_pos_lookups++;
  }
  std::cerr << "n_pos_lookups: " << n_pos_lookups << std::endl;
  std::cerr << "n_tuples: " << n_tuples << std::endl;

  delete(c);

  return 0;
}
