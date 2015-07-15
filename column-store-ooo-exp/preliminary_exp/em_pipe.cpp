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
  cstore::DS2 *ds2 = new cstore::DS2(c1, pred1, col1_compare_type);
  cstore::DS4 *ds4 = new cstore::DS4(c2, pred2, col2_compare_type);

  bool ret;
  bool passed;
  char *val1, *val2;
  off_t pos;
  off_t start, length;
  off_t n_pos_lookups = 0;
  off_t n_tuples = 0;
  if (c1->get_comp_type() == cstore::RLE &&
      c2->get_comp_type() == cstore::RLE) {
    while (true) {
      // optimized version
      /*
      ret = ds2->rle_op(&start, &length, (void **) &val1);
      if (!ret) { break; }
      std::deque<cstore::rle_triple_tuple_t *> vs;
      ret = ds4->rle_op(start, length, vs);
      for (int i = 0; i < vs.size(); ++i) {
        cstore::rle_triple_tuple_t *t = vs.at(i);
        val2 = t->val;
        for (int j = 0; j < t->length; ++j) {
          n_tuples++;
          //cstore::output_tuple(val1, c1, val2, c2);
        }
      }
      */

      // paper version
      ret = ds2->rle_op(&pos, (void **) &val1);
      if (!ret) { break; }
      ret = ds4->rle_op_new(pos, (void **) &val2, &passed);
      n_pos_lookups++;
      if (passed) {
        n_tuples++;
        //cstore::output_tuple(val1, c1, val2, c2);
      }
    }
  } else if (c1->get_comp_type() == cstore::RLE &&
             c2->get_comp_type() == cstore::NOCOMP) {
    while (true) {
      ret = ds2->rle_op(&pos, (void **) &val1);
      if (!ret) { break; }
      //ret = ds4->nocomp_op(pos, (void **) &val2, &passed);
      ret = ds4->nocomp_op_new(pos, (void **) &val2, &passed);
      n_pos_lookups++;
      if (passed) {
        n_tuples++;
        //cstore::output_tuple(val1, c1, val2, c2);
      }
    }

    // blocking version
    /*
    while (true) {
      std::deque<off_t> poss;
      std::deque<void *> val1s;
      std::deque<void *> val2s;
      ret = ds2->rle_op_block(poss, val1s);
      if (poss.empty()) { break; }
      ret = ds4->nocomp_op_block(poss, val2s);
      n_pos_lookups += poss.size();
      if (!val2s.empty()) {
        n_tuples += val2s.size();
        //cstore::output_tuple(val1, c1, val2, c2);
      }
    }
    */
  } else if (c1->get_comp_type() == cstore::NOCOMP &&
             c2->get_comp_type() == cstore::NOCOMP) {
    while (true) {
      ret = ds2->nocomp_op(&pos, (void **) &val1);
      if (!ret) { break; }
      ret = ds4->nocomp_op_new(pos, (void **) &val2, &passed);
      n_pos_lookups++;
      if (passed) {
        n_tuples++;
        //cstore::output_tuple(val1, c1, val2, c2);
      }
    }
  }
  std::cerr << "n_pos_lookups: " << n_pos_lookups << std::endl;
  std::cerr << "n_tuples: " << n_tuples << std::endl;

  delete(c1);
  delete(c2);

  return 0;
}
