#include <iostream>
#include "ops.h"
#include "query_util.h"

int main(int argc, char *argv[]) {

  if (argc != 7) {
    std::cerr << "Usage: " << argv[0] << " col1_name col2_name col1_pred col2_pred col1_compare col2_compare" << std::endl;
    exit(1);
  }
  char *col1_name = argv[1];
  char *col2_name = argv[2];
  char *col1_pred = argv[3];
  char *col2_pred = argv[4];
  cstore::compare_t  col1_compare_type = cstore::get_compare_type(argv[5]);
  cstore::compare_t  col2_compare_type = cstore::get_compare_type(argv[6]);

  cstore::Column *c1 = new cstore::Column();
  c1->open(col1_name, COL_RDONLY);

  cstore::Column *c2 = new cstore::Column();
  c2->open(col2_name, COL_RDONLY);

  void *pred1 = cstore::get_pred(col1_pred, c1->get_col_type());
  void *pred2 = cstore::get_pred(col2_pred, c2->get_col_type());

  cstore::SPC2 *spc = new cstore::SPC2(c1, pred1, col1_compare_type, c2, pred2, col2_compare_type);

  bool ret;
  bool passed;
  char *val1, *val2;
  off_t pos;
  off_t n_pos_lookups = 0;
  off_t n_tuples = 0;
  while (true) {
    ret = spc->op((void **) &val1, (void **) &val2, &passed);
    if (!ret) {
      break;
    }
    if (passed) {
      n_tuples++;
      //cstore::output_tuple(val1, c1, val2, c2);
    }
  }
  std::cerr << "n_pos_lookups: " << n_pos_lookups << std::endl;
  std::cerr << "n_tuples: " << n_tuples << std::endl;

  delete(c1);
  delete(c2);

  return 0;
}
