#include <iostream>
#include <vector>
#include "ops.h"
#include "query_util.h"
#include <math.h>

int main(int argc, char *argv[]) {

  if (argc != 11) {
    std::cerr << "Usage: " << argv[0] << " col1_name col2_name col1_pred col2_pred col1_compare col2_compare ";
    std::cerr << "#_of_other_1ms_samples #_of_other_10ms_samples #_of_other_100ms_samples #_of_other_1000ms_samples" << std::endl;
    exit(1);
  }
  char *col1_name = argv[1];
  char *col2_name = argv[2];
  char *col1_pred = argv[3];
  char *col2_pred = argv[4];
  cstore::compare_t  col1_compare_type = cstore::get_compare_type(argv[5]);
  cstore::compare_t  col2_compare_type = cstore::get_compare_type(argv[6]);
  int num_mss[4];
  num_mss[0] = atoi(argv[7]); // 1ms
  num_mss[1] = atoi(argv[8]); // 10ms
  num_mss[2] = atoi(argv[9]); // 100ms
  num_mss[3] = atoi(argv[10]); // 1000ms
  int num_sensors = num_mss[0] + num_mss[1] + num_mss[2] + num_mss[3];

  std::vector<cstore::DS4 *> dsv;

  cstore::Column *c1 = new cstore::Column();
  c1->open(col1_name, COL_RDONLY);
  cstore::Column *c2 = new cstore::Column();
  c2->open(col2_name, COL_RDONLY);

  void *pred1 = NULL;
  if (strcmp(col1_pred, "NULL") != 0) {
    pred1 = cstore::get_pred(col1_pred, c1->get_col_type());
  }
  void *pred2 = NULL;
  if (strcmp(col2_pred, "NULL") != 0) {
    pred2 = cstore::get_pred(col2_pred, c2->get_col_type());
  }
  cstore::DS2 *ds2 = new cstore::DS2(c1, pred1, col1_compare_type);
  dsv.push_back((cstore::DS4 *) NULL);

  cstore::DS4 *ds4 = new cstore::DS4(c2, pred2, col2_compare_type);
  dsv.push_back(ds4);

  for (int j = 0; j < 4; ++j) {
    for (int i = 0; i < num_mss[j]; ++i) {
      cstore::Column *c3= new cstore::Column();
      char buf[128];
      sprintf(buf, "./samples/sample%d.%d.col", (int) pow(10, j), i+3);
      c3->open(buf, COL_RDONLY);
      cstore::DS4 *ds4s1 = new cstore::DS4(c3, NULL, cstore::EQ);
      dsv.push_back(ds4s1);
    }
  }

  bool ret;
  bool passed;
  char *val1, *val2, *val3;
  off_t pos;
  off_t start, length;
  off_t n_pos_lookups = 0;
  off_t n_tuples = 0;
  if (c1->get_comp_type() == cstore::RLE &&
      c2->get_comp_type() == cstore::RLE) {
    while (true) {
      // paper version
      ret = ds2->rle_op(&pos, (void **) &val1);
      if (!ret) { break; }

      ret = ds4->rle_op_new(pos, (void **) &val2, &passed);
      n_pos_lookups++;
      if (!passed) { continue; }
      //std::cout << "passed" << std::endl;
      // fetch column value from other sensor columns

      for (int i = 0; i < num_sensors; ++i) {
        cstore::DS4 *ds = dsv.at(i+2);
        if (ds->get_comp_type() == cstore::RLE) {
          ret = ds->rle_op_new(pos, (void **) &val3, &passed);
        } else {
          ret = ds->nocomp_op_new(pos, (void **) &val3, &passed);
        }
      }
      n_tuples++;
      //cstore::output_tuple(val1, c1, val2, c2);
    }
  } else if (c1->get_comp_type() == cstore::RLE &&
             c2->get_comp_type() == cstore::NOCOMP) {
    while (true) {
      ret = ds2->rle_op(&pos, (void **) &val1);
      if (!ret) { break; }

      //ret = ds4->nocomp_op(pos, (void **) &val2, &passed);
      ret = ds4->nocomp_op_new(pos, (void **) &val2, &passed);
      if (!passed) {
        continue;
      }
      
      // fetch column value from other sensor columns
      for (int i = 0; i < num_sensors; ++i) {
        cstore::DS4 *ds = dsv.at(i+2);
        if (ds->get_comp_type() == cstore::RLE) {
          ret = ds->rle_op_new(pos, (void **) &val3, &passed);
        } else {
          ret = ds->nocomp_op_new(pos, (void **) &val3, &passed);
        }
      }
      n_tuples++;
      //cstore::output_tuple(val1, c1, val2, c2);
    }
  } else if (c1->get_comp_type() == cstore::NOCOMP &&
             c2->get_comp_type() == cstore::NOCOMP) {
    while (true) {
      ret = ds2->nocomp_op(&pos, (void **) &val1);
      if (!ret) { break; }

      //std::cout << "val1: " << *(int64_t *) val1 << std::endl;
      //ret = ds4->nocomp_op(pos, (void **) &val2, &passed);
      ret = ds4->nocomp_op_new(pos, (void **) &val2, &passed);
      if (!passed) {
        continue;
      }

      // fetch column value from other sensor columns
      for (int i = 0; i < num_sensors; ++i) {
        cstore::DS4 *ds = dsv.at(i+2);
        if (ds->get_comp_type() == cstore::RLE) {
          ret = ds->rle_op_new(pos, (void **) &val3, &passed);
        } else {
          ret = ds->nocomp_op_new(pos, (void **) &val3, &passed);
        }
      }
      n_tuples++;
      //cstore::output_tuple(val1, c1, val2, c2);
      //delete(val1);
      //delete(val2);
      //delete(val3);
    }
  }
  std::cerr << "n_tuples: " << n_tuples << std::endl;

  delete(c1);
  delete(c2);

  return 0;
}
