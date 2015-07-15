#include <iostream>
#include <sys/time.h>
#include "ops.h"
#include "query_util.h"

static double gettimeofday_sec();

int main(int argc, char *argv[]) {

  if (argc != 5) {
    std::cerr << "Usage: " << argv[0] << " col_name col_pred col_compare stride_size" << std::endl;
    exit(1);
  }
  char *col_name = argv[1];
  char *col_pred = argv[2];
  cstore::compare_t  col_compare_type = cstore::get_compare_type(argv[3]);
  off_t stride_size = atoll(argv[4]);

  cstore::Column *c = new cstore::Column();
  if (!c->open(col_name, COL_RDONLY)) {
    std::cerr << "open failed." << std::endl;
    exit(1);
  }

  double t1 = gettimeofday_sec();
  void *pred = NULL;
  if (strcmp(col_pred, "NULL") != 0) {
    pred = cstore::get_pred(col_pred, c->get_col_type());
  }
  cstore::DS4 *ds4 = new cstore::DS4(c, pred, col_compare_type);

  bool ret;
  bool passed;
  char *val1;
  off_t pos = 0;
  off_t n_pos_lookups = 0;
  off_t n_total = 0;
  off_t n_tuples = c->get_n_tuples();

  while (pos < n_tuples) {
    ret = ds4->nocomp_op_new(pos, (void **) &val1, &passed);
    n_total++;
    if (passed) {
      n_pos_lookups++;
    }
    pos += stride_size;
  }
  std::cerr << "n_pos_lookups: " << n_pos_lookups << std::endl;
  std::cerr << "n_total: " << n_total << std::endl;
  std::cerr << "n_tuples: " << n_tuples << std::endl;
  double t2 = gettimeofday_sec();
  std::cerr << "time : " << t2 - t1 << std::endl;

  delete(c);

  return 0;
}

static double 
gettimeofday_sec()
{
  struct timeval tv; 
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}
