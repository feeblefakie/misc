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

  cstore::Column *c1_2 = new cstore::Column();
  c1_2->open(col1_name, COL_RDONLY);
  cstore::Column *c2_2 = new cstore::Column();
  c2_2->open(col2_name, COL_RDONLY);

  void *pred1 = cstore::get_pred(col1_pred, c1->get_col_type());
  void *pred2 = cstore::get_pred(col2_pred, c2->get_col_type());
  cstore::DS1 *ds1 = new cstore::DS1(c1, pred1, col1_compare_type);
  cstore::DS3 *ds3 = new cstore::DS3(c2);
  cstore::DS1a *ds1a = new cstore::DS1a(c2, pred2, col2_compare_type);
  cstore::DS3 *ds3_1 = new cstore::DS3(c1_2);
  cstore::DS3 *ds3_2 = new cstore::DS3(c2_2);

  bool ret;
  bool passed;
  char *val1, *val2;
  off_t pos;
  off_t n_pos_lookups = 0;
  off_t n_tuples = 0;
  if (c1->get_comp_type() == cstore::RLE &&
      c2->get_comp_type() == cstore::RLE) {
    while (true) {
      off_t start, length;
      ret = ds1->rle_op(&start, &length);
      if (!ret) { break; }
      std::deque<cstore::rle_triple_tuple_t *> vs;
      ret = ds3->rle_op(start, length, vs);
      std::deque<cstore::rle_triple_tuple_t *> vs1;
      std::deque<cstore::rle_triple_tuple_t *> vs2;
      for (int i = 0; i < vs.size(); ++i) {
        cstore::rle_triple_tuple_t *t = vs.at(i);
        ret = ds1a->rle_op(t, &passed);
        if (!passed) { continue; }
        ds3_1->rle_op(t->start, t->length, vs1);
        ds3_2->rle_op(t->start, t->length, vs2);
        // extended merge operator
        int o1 = 0;
        int o2 = 0;
        cstore::rle_triple_tuple_t *vs1cur = vs1.at(o1);
        cstore::rle_triple_tuple_t *vs2cur = vs2.at(o2);
        for (int j = t->start; j < t->start + t->length; ++j) {
          if (j >= vs1cur->start && j < vs1cur->start + vs1cur->length) {
            val1 = vs1cur->val;
          } else {
            vs1cur = vs1.at(++o1);
            val1 = vs1cur->val;
          }
          if (j >= vs2cur->start && j < vs2cur->start + vs2cur->length) {
            val2 = vs2cur->val;
          } else {
            vs2cur = vs2.at(++o2);
            val2 = vs2cur->val;
          }
          n_tuples++;
          //cstore::output_tuple(val1, c1_2, val2, c2_2);
        }
        //std::cout << "hello" << std::endl;
      }
      std::deque<cstore::rle_triple_tuple_t *>::iterator itr = vs.begin();
      for (; itr != vs.end(); ++itr) {
        delete(*itr);
      }
      itr = vs1.begin();
      for (; itr != vs1.end(); ++itr) {
        delete(*itr);
      }
      itr = vs2.begin();
      for (; itr != vs2.end(); ++itr) {
        delete(*itr);
      }
      /*
      // old version
      ret = ds1->rle_op(&pos);
      ret = ds3->rle_op(pos, (void **) &val2);
      ret = ds1a->nocomp_op(val2, &passed);
      if (!passed) { continue; }
      ret = ds3_1->rle_op(pos, (void **) &val1);
      ret = ds3_2->rle_op(pos, (void **) &val2);
      n_tuples++;
      */
      //cstore::output_tuple(val1, c1_2, val2, c2_2);
    }
  } else if (c1->get_comp_type() == cstore::RLE &&
             c2->get_comp_type() == cstore::NOCOMP) {
    while (true) {
      off_t start, length;
      ret = ds1->rle_op(&start, &length);
      if (!ret) { break; }
      std::deque<cstore::nocomp_tuple_t *> vs;
      ret = ds3->nocomp_op(start, length, vs);
      off_t pos = start;
      off_t prev_pos = start;
      for (int i = 0; pos < start + length; ++i, ++pos) {
        cstore::nocomp_tuple_t *t = vs.at(i);
        ret = ds1a->nocomp_op(t, &passed);
        if (!passed) { 
          if (prev_pos != pos) {
            std::deque<cstore::rle_triple_tuple_t *> vs1;
            std::deque<cstore::nocomp_tuple_t *> vs2;
            ds3_1->rle_op(prev_pos, pos-prev_pos, vs1);
            ds3_2->nocomp_op(prev_pos, pos-prev_pos, vs2);
            // TODO: merge
            n_tuples += vs2.size();
            for (std::deque<cstore::rle_triple_tuple_t *>::iterator itr = vs1.begin();
                 itr != vs1.end(); ++itr) {
              delete(*itr);
            }
            for (std::deque<cstore::nocomp_tuple_t *>::iterator itr = vs2.begin();
                 itr != vs2.end(); ++itr) {
              delete(*itr);
            }
          }
          continue;
        }
        /*
        ds3_1->rle_op(pos, (void **) &val1);
        ds3_2->nocomp_op(pos, (void **) &val2);
        n_tuples++;
        */
        //cstore::output_tuple(val1, c1_2, val2, c2_2);
      }
      if (prev_pos != pos) {
        std::deque<cstore::rle_triple_tuple_t *> vs1;
        std::deque<cstore::nocomp_tuple_t *> vs2;
        ds3_1->rle_op(prev_pos, pos-prev_pos, vs1);
        ds3_2->nocomp_op(prev_pos, pos-prev_pos, vs2);
        // TODO: merge
        n_tuples += vs2.size();
        for (std::deque<cstore::rle_triple_tuple_t *>::iterator itr = vs1.begin();
             itr != vs1.end(); ++itr) {
          delete(*itr);
        }
        for (std::deque<cstore::nocomp_tuple_t *>::iterator itr = vs2.begin();
             itr != vs2.end(); ++itr) {
          delete(*itr);
        }
      }

      std::deque<cstore::nocomp_tuple_t *>::iterator itr = vs.begin();
      for (; itr != vs.end(); ++itr) {
        delete(*itr);
      }
      /*
      // old version
      ret = ds1->rle_op(&pos);
      if (!ret) { break; }
      ret = ds3->nocomp_op(pos, (void **) &val2);
      ret = ds1a->nocomp_op(val2, &passed);
      if (!passed) { continue; }
      ret = ds3_1->rle_op(pos, (void **) &val1);
      ret = ds3_2->nocomp_op(pos, (void **) &val2);
      n_tuples++;
      //cstore::output_tuple(val1, c1_2, val2, c2_2);
      */
    }
  }
  std::cerr << "n_pos_lookups: " << n_pos_lookups << std::endl;
  std::cerr << "n_tuples: " << n_tuples << std::endl;

  delete(c1);
  delete(c2);

  return 0;
}
