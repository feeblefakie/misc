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
  cstore::DS1 *ds1_1 = new cstore::DS1(c1, pred1, col1_compare_type);
  cstore::DS1 *ds1_2 = new cstore::DS1(c2, pred2, col2_compare_type);
  cstore::DS3 *ds3_1 = new cstore::DS3(c1_2);
  cstore::DS3 *ds3_2 = new cstore::DS3(c2_2);

  bool ret1, ret2;
  bool read_next1 = true;
  bool read_next2 = true;
  bool passed;
  char *val1, *val2;
  off_t pos;
  off_t n_pos_lookups = 0;
  off_t n_tuples = 0;
  off_t start1, length1;
  off_t start2, length2;
  if (c1->get_comp_type() == cstore::RLE &&
      c2->get_comp_type() == cstore::RLE) {
    while (true) {
      if (read_next1) {
        ret1 = ds1_1->rle_op(&start1, &length1);
        if (!ret1) { break; }
      }
      if (read_next2) {
        ret2 = ds1_2->rle_op(&start2, &length2);
        if (!ret2) { break; }
      }

      off_t start, length;
      // aneded
      if (start1 <= start2) {
        if (start1 + length1 > start2 + length2) {
          // area2 is included in area1
          start = start2;
          length = length2;
          // ds1_2 is goint to next
          read_next1 = false;
          read_next2 = true;
        } else if (start1 + length1 < start2 + length2) {
          start = start2;
          length = start1 + length1 - start2;
          // ds1_1 is going to next
          read_next1 = true;
          read_next2 = false;
        } else {
          // area2 is included in area1
          start = start2;
          length = length2;
          // ds1_1 and ds1_2 is going to next
          read_next1 = true;
          read_next2 = true;
        }
      } else {
        if (start2 + length2 > start1 + length1) {
          // area1 is included in area2
          start = start1;
          length = length1;
          // ds1_1 is going to next
          read_next1 = true;
          read_next2 = false;
        } else if (start2 + length2 < start1 + length1) {
          start = start1;
          length = start2 + length2 - start1;
          // ds1_2 is going to next
          read_next1 = false;
          read_next2 = true;
        } else {
          // area1 is included in area2
          start = start1;
          length = length1;
          // ds1_1 and ds1_2 is going to next
          read_next1 = true;
          read_next2 = true;
        }
      }

      std::deque<cstore::rle_triple_tuple_t *> vs1;
      std::deque<cstore::rle_triple_tuple_t *> vs2;

      ds3_1->rle_op(start, length, vs1);
      ds3_2->rle_op(start, length, vs2);
      // extended merge operator
      int o1 = 0;
      int o2 = 0;
      cstore::rle_triple_tuple_t *vs1cur = vs1.at(o1);
      cstore::rle_triple_tuple_t *vs2cur = vs2.at(o2);
      for (int j = start; j < start + length; ++j) {
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
      std::deque<cstore::rle_triple_tuple_t *>::iterator itr = vs1.begin();
      for (; itr != vs1.end(); ++itr) {
        delete(*itr);
      }
      itr = vs2.begin();
      for (; itr != vs2.end(); ++itr) {
        delete(*itr);
      }
    }
  } else if (c1->get_comp_type() == cstore::RLE &&
             c2->get_comp_type() == cstore::NOCOMP) {
    std::deque<off_t> poslist;
    std::deque<off_t> overflows;
    while (true) {
      poslist.clear();
      if (!overflows.empty()) {
        poslist.push_back(overflows.front());
        overflows.pop_front();
      }
      ret1 = ds1_1->rle_op(&start1, &length1);
      if (!ret1) {
        break;
      }
      //if (!ret1) { break; }
      off_t pos;
      while (true) {
        ret2 = ds1_2->nocomp_op(&pos);
        //std::cout << "start1:" << start1 << ", length1:" << length1 << ", pos: " << pos << std::endl;
        //if (!ret2) { break; }
        if (pos >= start1 + length1 - 1) {
          if (pos > start1 + length1 - 1) {
            overflows.push_back(pos);
          } else {
            poslist.push_back(pos);
          }
          break;
        } else if (pos < start1) {
          continue;
        }
        poslist.push_back(pos);
      }
      // intersection betweeen [start1,start1+lenght1) and poslist

      std::deque<cstore::rle_triple_tuple_t *> vs1;
      std::deque<cstore::nocomp_tuple_t *> vs2;

      if (!poslist.empty()) {
        ds3_1->rle_op(poslist, vs1);
        ds3_2->nocomp_op(poslist, vs2);
        // extended merge operator
        int o1 = 0;
        int o2 = 0;
        cstore::rle_triple_tuple_t *vs1cur = vs1.at(o1);
        cstore::nocomp_tuple_t *vs2cur = vs2.at(o2);
        //std::cout << "vs2 size: " << vs2.size() << std::endl;
        //std::cout << "length: " << length << std::endl;
        for (int j = vs1cur->start; j < vs1cur->start + vs1cur->length - 1; ++j) {
          if (j >= vs1cur->start && j < vs1cur->start + vs1cur->length) {
            val1 = vs1cur->val;
          } else {
            vs1cur = vs1.at(++o1);
            val1 = vs1cur->val;
          }
          if (++o2 >= vs2.size()) {
            break;
          }
          val2 = vs2.at(o2)->val;
          n_tuples++;
          //cstore::output_tuple(val1, c1_2, val2, c2_2);
        }
      }
      /*
      std::deque<cstore::rle_triple_tuple_t *>::iterator itr1 = vs1.begin();
      for (; itr1 != vs1.end(); ++itr1) {
        delete(*itr1);
      }
      std::deque<cstore::nocomp_tuple_t *>::iterator itr2 = vs2.begin();
      for (; itr2 != vs2.end(); ++itr2) {
        delete(*itr2);
      }
      */
      if (!ret1 && !ret2) {
        break;
      }
    }
  }
  std::cerr << "n_pos_lookups: " << n_pos_lookups << std::endl;
  std::cerr << "n_tuples: " << n_tuples << std::endl;

  delete(c1);
  delete(c2);

  return 0;
}
