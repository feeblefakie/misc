#include <haildb.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <sys/time.h>

// intermediate value
typedef struct {
  uint32_t pos;
  uint32_t val;
} im_val_t;

typedef struct {
  uint32_t agg;
} im_agg_t;

char db_name[32] = "/data/cstore";
// query predicates
uint32_t pred_c_nationkey = 10;
uint32_t pred_o_orderdate = 8036;
uint32_t pred_l_shipdate = 8036;
ib_crsr_t crsr_l1_s, crsr_l1_e, crsr_l1_o, crsr_l3_o, crsr_l3_c, crsr_l3_ok, crsr_l5_c, crsr_l5_n, crsr_l6_o, crsr_l6_s, crsr_l6_e, crsr_ji;
ib_tpl_t tpl_l1_s, tpl_l1_e, tpl_l1_o, tpl_l3_o, tpl_l3_c, tpl_l3_ok, tpl_l5_c, tpl_l5_n, tpl_l6_o, tpl_l6_s, tpl_l6_e, tpl_ji;
bool use_l1 = false;

bool init_db(void);
bool fin_db(void);
void q3(void);
void scan(ib_crsr_t crsr, ib_tpl_t tpl);
std::vector<im_val_t> get_l3_orderkeys_from_positions(std::vector<uint32_t> pos_array);
std::vector<uint32_t> apply_l5_predicate(std::vector<im_val_t> pos_custkey_array);
std::vector<uint32_t> apply_l5_predicate_scan(std::vector<im_val_t> pos_custkey_array);
std::vector<im_val_t> get_l3_custkeys_from_sequences(int pos, int freq);
std::vector<im_agg_t> apply_l1_predicate_via_ji(std::vector<im_val_t> pos_orderkey_array);
std::vector<im_agg_t> apply_l6_predicate(std::vector<im_val_t> pos_orderkey_array);
void apply_l1_predicate_and_scan(uint32_t pred_shipdate);

bool val_asc(const im_val_t& l, const im_val_t& r) {
  return l.val < r.val;
}

static double 
gettimeofday_sec()
{
  struct timeval tv; 
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

int main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << argv[0] << " orderdate use_l1(1/0)" << std::endl;
    exit(1);
  }
  uint32_t orderdate = atoi(argv[1]);
  pred_o_orderdate = orderdate;
  pred_l_shipdate = orderdate;
  if (atoi(argv[2]) == 1) {
    use_l1 = true;
  } else {
    use_l1 = false;
  }
  if (!init_db()) { exit(1); }

  q3();

  fin_db();

  return 0;
}

void q3(void)
{
  char *l1_shipdate_rle = "l1_shipdate_rle";
  char *l1_extendedprice_uncompressed = "l1_extendedprice_uncompressed";
  char *l1_orderkey_uncompressed = "l1_orderkey_uncompressed";
  char *l3_orderdate_rle = "l3_orderdate_rle";
  char *l3_custkey_uncompressed = "l3_custkey_uncompressed";
  char *l3_orderkey_uncompressed = "l3_orderkey_uncompressed";
  char *l5_customer_key_rle = "l5_customer_key_rle";
  char *l5_nationkey_uncompressed = "l5_nationkey_uncompressed";
  char *l6_orderkey_rle = "l6_orderkey_rle";
  char *l6_extendedprice_uncompressed = "l6_extendedprice_uncompressed";
  char *l6_shipdate_uncompressed = "l6_shipdate_uncompressed";
  char *ji_l3_l1 = "ji_l3-l1";

  char cstore_l1_shipdate_rle[64];
  char cstore_l1_extendedprice_uncompressed[64];
  char cstore_l1_orderkey_uncompressed[64];
  char cstore_l3_orderdate_rle[64];
  char cstore_l3_custkey_uncompressed[64];
  char cstore_l3_orderkey_uncompressed[64];
  char cstore_l5_customer_key_rle[64];
  char cstore_l5_nationkey_uncompressed[64];
  char cstore_l6_orderkey_rle[64];
  char cstore_l6_extendedprice_uncompressed[64];
  char cstore_l6_shipdate_uncompressed[64];
  char cstore_ji_l3_l1[64];
  sprintf(cstore_l1_shipdate_rle, "%s/%s", db_name, l1_shipdate_rle);
  sprintf(cstore_l1_extendedprice_uncompressed, "%s/%s", db_name, l1_extendedprice_uncompressed);
  sprintf(cstore_l1_orderkey_uncompressed, "%s/%s", db_name, l1_orderkey_uncompressed);
  sprintf(cstore_l3_orderdate_rle, "%s/%s", db_name, l3_orderdate_rle);
  sprintf(cstore_l3_custkey_uncompressed, "%s/%s", db_name, l3_custkey_uncompressed);
  sprintf(cstore_l3_orderkey_uncompressed, "%s/%s", db_name, l3_orderkey_uncompressed);
  sprintf(cstore_l5_customer_key_rle, "%s/%s", db_name, l5_customer_key_rle);
  sprintf(cstore_l5_nationkey_uncompressed, "%s/%s", db_name, l5_nationkey_uncompressed);
  sprintf(cstore_l6_orderkey_rle, "%s/%s", db_name, l6_orderkey_rle);
  sprintf(cstore_l6_extendedprice_uncompressed, "%s/%s", db_name, l6_extendedprice_uncompressed);
  sprintf(cstore_l6_shipdate_uncompressed, "%s/%s", db_name, l6_shipdate_uncompressed);
  sprintf(cstore_ji_l3_l1, "%s/%s", db_name, ji_l3_l1);

  double t0 = gettimeofday_sec();
  ib_err_t err;
  //ib_crsr_t crsr_l1_s, crsr_l1_e, crsr_l3_o, crsr_l3_c, crsr_l5_c, crsr_l5_n, crsr_ji;

  ib_trx_t ib_trx = ib_trx_begin(IB_TRX_REPEATABLE_READ);
  err = ib_cursor_open_table(cstore_l1_shipdate_rle, ib_trx, &crsr_l1_s);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_l1_shipdate_rle << std::endl; }
  err = ib_cursor_open_table(cstore_l1_extendedprice_uncompressed, ib_trx, &crsr_l1_e);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_l1_extendedprice_uncompressed << std::endl; }
  err = ib_cursor_open_table(cstore_l1_orderkey_uncompressed, ib_trx, &crsr_l1_o);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_l1_orderkey_uncompressed << std::endl; }
  err = ib_cursor_open_table(cstore_l3_orderdate_rle, ib_trx, &crsr_l3_o);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_l3_orderdate_rle << std::endl; }
  err = ib_cursor_open_table(cstore_l3_custkey_uncompressed, ib_trx, &crsr_l3_c);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_l3_custkey_uncompressed << std::endl; }
  err = ib_cursor_open_table(cstore_l3_orderkey_uncompressed, ib_trx, &crsr_l3_ok);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_l3_orderkey_uncompressed << std::endl; }
  err = ib_cursor_open_table(cstore_l5_customer_key_rle, ib_trx, &crsr_l5_c);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_l5_customer_key_rle << std::endl; }
  err = ib_cursor_open_table(cstore_l5_nationkey_uncompressed, ib_trx, &crsr_l5_n);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_l5_nationkey_uncompressed << std::endl; }
  err = ib_cursor_open_table(cstore_l6_orderkey_rle, ib_trx, &crsr_l6_o);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_l6_orderkey_rle << std::endl; }
  err = ib_cursor_open_table(cstore_l6_extendedprice_uncompressed, ib_trx, &crsr_l6_e);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_l6_extendedprice_uncompressed << std::endl; }
  err = ib_cursor_open_table(cstore_l6_shipdate_uncompressed, ib_trx, &crsr_l6_s);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_l6_shipdate_uncompressed << std::endl; }
  err = ib_cursor_open_table(cstore_ji_l3_l1, ib_trx, &crsr_ji);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_ji_l3_l1 << std::endl; }

  tpl_l1_s = ib_clust_read_tuple_create(crsr_l1_s);
  tpl_l1_e = ib_clust_read_tuple_create(crsr_l1_e);
  tpl_l1_o = ib_clust_read_tuple_create(crsr_l1_o);
  tpl_l3_o = ib_clust_read_tuple_create(crsr_l3_o);
  tpl_l3_c = ib_clust_read_tuple_create(crsr_l3_c);
  tpl_l3_ok = ib_clust_read_tuple_create(crsr_l3_ok);
  tpl_l5_c = ib_clust_read_tuple_create(crsr_l5_c);
  tpl_l5_n = ib_clust_read_tuple_create(crsr_l5_n);
  tpl_l6_o = ib_clust_read_tuple_create(crsr_l6_o);
  tpl_l6_e = ib_clust_read_tuple_create(crsr_l6_e);
  tpl_l6_s = ib_clust_read_tuple_create(crsr_l6_s);
  tpl_ji = ib_clust_read_tuple_create(crsr_ji);

  // scan L3.o_orderdate
  int res;
  err = ib_cursor_first(crsr_l3_o);
  if (err == DB_SUCCESS) {
    //std::cout << "success" << std::endl;
  } else {
    std::cout << "failed" << std::endl;
    ib_cursor_close(crsr_l3_o); 
    return;
  }

  while (err == DB_SUCCESS) {
    err = ib_cursor_read_row(crsr_l3_o, tpl_l3_o);
    /* Possible handle locking and timeout errors too in multi-threaded applications. */
    if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
      break;
    }
    ib_u32_t orderdate;
    ib_tuple_read_u32(tpl_l3_o, 0, &orderdate);
    if (orderdate >= pred_o_orderdate) { break; }
    err = ib_cursor_next(crsr_l3_o);
    /* Possible handle locking and timeout errors too
    *  in multi-threaded applications. */
    if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
      break;
    }
    tpl_l3_o = ib_tuple_clear(tpl_l3_o);
  }

  scan(crsr_l3_c, tpl_l3_c);
  scan(crsr_l5_c, tpl_l5_c);
  scan(crsr_l5_n, tpl_l5_n);
  scan(crsr_l3_ok, tpl_l3_ok);
  /*
  if (use_l1) {
    // predicate can be applied to scan less
    apply_l1_predicate_and_scan(pred_o_orderdate);
  } else {
    scan(crsr_l1_o, tpl_l1_o);
    scan(crsr_l1_s, tpl_l1_s);
  }
  */

  ib_tuple_delete(tpl_l3_o);
  ib_cursor_close(crsr_l1_s);
  ib_cursor_close(crsr_l1_e);
  ib_cursor_close(crsr_l1_o);
  ib_cursor_close(crsr_l3_o); 
  ib_cursor_close(crsr_l3_c); 
  ib_cursor_close(crsr_l3_ok); 
  ib_cursor_close(crsr_l5_c); 
  ib_cursor_close(crsr_l5_n); 
  ib_cursor_close(crsr_l6_o); 
  ib_cursor_close(crsr_l6_e); 
  ib_cursor_close(crsr_l6_s); 
  ib_cursor_close(crsr_ji); 
  ib_trx_commit(ib_trx);
  double t6 = gettimeofday_sec();
  std::cout << "orderdate: " << pred_o_orderdate << " total:" << t6-t0 << std::endl;
}

void
scan(ib_crsr_t crsr, ib_tpl_t tpl)
{
  int res;
  ib_err_t err;
  err = ib_cursor_first(crsr);
  if (err == DB_SUCCESS) {
    //std::cout << "success" << std::endl;
  } else {
    std::cout << "failed" << std::endl;
    ib_cursor_close(crsr); 
    return;
  }

  while (err == DB_SUCCESS) {
    err = ib_cursor_read_row(crsr, tpl);
    /* Possible handle locking and timeout errors too in multi-threaded applications. */
    if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
      break;
    }
    err = ib_cursor_next(crsr);
    /* Possible handle locking and timeout errors too
    *  in multi-threaded applications. */
    if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
      break;
    }
    tpl = ib_tuple_clear(tpl);
  }
}

void
apply_l1_predicate_and_scan(uint32_t pred_shipdate)
{
  ib_err_t err;
  ib_crsr_t index_crsr;
  err = ib_cursor_open_index_using_name(crsr_l1_s, "SECONDARY_KEY", &index_crsr);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << std::endl; }
  ib_cursor_set_cluster_access(index_crsr);
  ib_tpl_t key_l1_s = ib_sec_search_tuple_create(index_crsr);
  ib_tpl_t key_l1_o = ib_clust_search_tuple_create(crsr_l6_s);
  int res;
  ib_tuple_write_u32(key_l1_s, 0, pred_shipdate);
  err = ib_cursor_moveto(index_crsr, key_l1_s, IB_CUR_G, &res);
  if (err != DB_SUCCESS) {
    std::cout << "failed moving cursor for index_crsr" << std::endl;
  }
  err = ib_cursor_read_row(index_crsr, tpl_l1_s);
  if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
  }
  ib_u32_t shipdate, pos;
  ib_tuple_read_u32(tpl_l1_s, 0, &shipdate);
  ib_tuple_read_u32(tpl_l1_s, 1, &pos);
  tpl_l1_s = ib_tuple_clear(tpl_l1_s);
  std::cout << "l1-pos: " << pos << std::endl;

  // scan l1_orderkey for HJ
  ib_tuple_write_u32(key_l1_o, 0, pos);
  err = ib_cursor_moveto(crsr_l1_o, key_l1_o, IB_CUR_GE, &res);
  while (err == DB_SUCCESS) {
    err = ib_cursor_read_row(crsr_l1_o, tpl_l1_o);
    err = ib_cursor_next(crsr_l1_o);
    tpl_l1_o = ib_tuple_clear(tpl_l1_o);
  }
  ib_tuple_delete(key_l1_s);
  ib_tuple_delete(key_l1_o);
  ib_cursor_close(index_crsr);
}

std::vector<im_agg_t>
apply_l6_predicate(std::vector<im_val_t> pos_orderkey_array)
{
  ib_err_t err;
  std::vector<im_agg_t> extendedprice_array;
  ib_crsr_t index_crsr;
  err = ib_cursor_open_index_using_name(crsr_l6_o, "SECONDARY_KEY", &index_crsr);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << std::endl; }
  ib_cursor_set_cluster_access(index_crsr);
  ib_tpl_t key_l6_o = ib_sec_search_tuple_create(index_crsr);
  ib_tpl_t key_l6_s = ib_clust_search_tuple_create(crsr_l6_s);
  ib_tpl_t key_l6_e = ib_clust_search_tuple_create(crsr_l6_e);
  size_t size = pos_orderkey_array.size();
  int res;
  for (int i = 0; i < size; ++i) {
    uint32_t sum_extendedprice = 0;
    ib_tuple_write_u32(key_l6_o, 0, pos_orderkey_array[i].val);
    err = ib_cursor_moveto(index_crsr, key_l6_o, IB_CUR_GE, &res);
    if (err != DB_SUCCESS) {
      std::cout << "failed moving cursor for index_crsr" << std::endl;
    }
    err = ib_cursor_read_row(index_crsr, tpl_l6_o);
    if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
    }
    ib_u32_t orderkey, pos, freq;
    ib_tuple_read_u32(tpl_l6_o, 0, &orderkey);
    ib_tuple_read_u32(tpl_l6_o, 1, &pos);
    ib_tuple_read_u32(tpl_l6_o, 2, &freq);
    //std::cout << "orderkey: " << orderkey << ", pos: " << pos << ", freq: " << freq << std::endl;
    if (orderkey != pos_orderkey_array[i].val) {
      break;
    }
    tpl_l6_o = ib_tuple_clear(tpl_l6_o);

    for (int i = pos; i < pos + freq; ++i) {
      ib_tuple_write_u32(key_l6_s, 0, i);
      err = ib_cursor_moveto(crsr_l6_s, key_l6_s, IB_CUR_GE, &res);
      if (err != DB_SUCCESS) {
        std::cout << "failed moving cursor for l6_s" << std::endl;
      }
      err = ib_cursor_read_row(crsr_l6_s, tpl_l6_s);
      ib_u32_t shipdate;
      ib_tuple_read_u32(tpl_l6_s, 1, &shipdate);
      //std::cout << "shipdate: " << shipdate << std::endl;
      if (shipdate > pred_l_shipdate) {
        ib_tuple_write_u32(key_l6_e, 0, i);
        err = ib_cursor_moveto(crsr_l6_e, key_l6_e, IB_CUR_GE, &res);
        if (err != DB_SUCCESS) {
          std::cout << "failed moving cursor for l6_e" << std::endl;
        }
        err = ib_cursor_read_row(crsr_l6_e, tpl_l6_e);
        ib_u32_t extendedprice;
        ib_tuple_read_u32(tpl_l6_e, 1, &extendedprice);
        sum_extendedprice += extendedprice;
        tpl_l6_e = ib_tuple_clear(tpl_l6_e);
      }
      tpl_l6_s = ib_tuple_clear(tpl_l6_s);
    }
    im_agg_t im_agg = {sum_extendedprice};
    extendedprice_array.push_back(im_agg);
  }
  ib_tuple_delete(key_l6_o);
  ib_tuple_delete(key_l6_e);
  ib_tuple_delete(key_l6_s);
  ib_cursor_close(index_crsr);

  return extendedprice_array;
}

std::vector<im_agg_t>
apply_l1_predicate_via_ji(std::vector<im_val_t> pos_orderkey_array)
{
  ib_err_t err;
  std::vector<im_agg_t> extendedprice_array;
  ib_tpl_t key_ji = ib_clust_search_tuple_create(crsr_ji);
  ib_tpl_t key_l1_s = ib_clust_search_tuple_create(crsr_l1_s);
  ib_tpl_t key_l1_e = ib_clust_search_tuple_create(crsr_l1_e);
  size_t size = pos_orderkey_array.size();
  int res;
  for (int i = 0; i < size; ++i) {
    uint32_t sum_extendedprice = 0;
    ib_tuple_write_u32(key_ji, 0, pos_orderkey_array[i].pos);
    err = ib_cursor_moveto(crsr_ji, key_ji, IB_CUR_GE, &res);
    if (err != DB_SUCCESS) {
      std::cout << "failed moving cursor for ji" << std::endl;
    }
    while (err == DB_SUCCESS) {
      err = ib_cursor_read_row(crsr_ji, tpl_ji);
      if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
      }
      ib_u32_t l3_pos, l1_pos;
      ib_tuple_read_u32(tpl_ji, 0, &l3_pos);
      ib_tuple_read_u32(tpl_ji, 1, &l1_pos);
      if (l3_pos != pos_orderkey_array[i].pos) {
        break;
      }
      ib_tuple_write_u32(key_l1_s, 0, l1_pos);
      err = ib_cursor_moveto(crsr_l1_s, key_l1_s, IB_CUR_LE, &res);
      if (err != DB_SUCCESS) {
        std::cout << "failed moving cursor for l1_s" << std::endl;
      }
      // getting shipdate RLE triple
      err = ib_cursor_read_row(crsr_l1_s, tpl_l1_s);
      ib_u32_t l_shipdate, pos, freq;
      ib_tuple_read_u32(tpl_l1_s, 0, &l_shipdate);
      if (l_shipdate > pred_l_shipdate) {
        ib_tuple_write_u32(key_l1_e, 0, l1_pos);
        err = ib_cursor_moveto(crsr_l1_e, key_l1_e, IB_CUR_GE, &res);
        if (err != DB_SUCCESS) {
          std::cout << "failed moving cursor for l1_s" << std::endl;
        }
        err = ib_cursor_read_row(crsr_l1_e, tpl_l1_e);
        ib_u32_t l_extendedprice;
        ib_tuple_read_u32(tpl_l1_e, 1, &l_extendedprice);
        sum_extendedprice += l_extendedprice;
      }
      err = ib_cursor_next(crsr_ji);
      tpl_ji = ib_tuple_clear(tpl_ji);
    }
    im_agg_t im_agg = {sum_extendedprice};
    extendedprice_array.push_back(im_agg);
  }
  ib_tuple_delete(key_ji);
  ib_tuple_delete(key_l1_s);

  return extendedprice_array;
}

std::vector<im_val_t>
get_l3_orderkeys_from_positions(std::vector<uint32_t> pos_array)
{
  ib_err_t err;
  std::vector<im_val_t> pos_orderkey_array;
  ib_tpl_t key = ib_clust_search_tuple_create(crsr_l3_ok);
  int res;
  size_t size = pos_array.size();
  // TODO: better to scan ?
  for (int i = 0; i < size; ++i) {
    ib_tuple_write_u32(key, 0, pos_array[i]);
    err = ib_cursor_moveto(crsr_l3_ok, key, IB_CUR_GE, &res);
    if (err != DB_SUCCESS) {
      std::cout << "failed moving cursor for l5_c" << std::endl;
    }
    if (res == 0) {
      err = ib_cursor_read_row(crsr_l3_ok, tpl_l3_ok);

      ib_u32_t orderkey;
      ib_tuple_read_u32(tpl_l3_ok, 1, &orderkey);
      //std::cout << "orderkey: " << orderkey << std::endl;
      im_val_t im_val = {pos_array[i], (uint32_t) orderkey};
      pos_orderkey_array.push_back(im_val);

      tpl_l3_ok = ib_tuple_clear(tpl_l3_ok);
    }
  }
  ib_tuple_delete(key);
  return pos_orderkey_array;
}

std::vector<uint32_t>
apply_l5_predicate(std::vector<im_val_t> pos_custkey_array)
{
  ib_err_t err;
  std::vector<uint32_t> pos_array;
  ib_crsr_t index_crsr;
  err = ib_cursor_open_index_using_name(crsr_l5_c, "SECONDARY_KEY", &index_crsr);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << std::endl; }
  ib_cursor_set_cluster_access(index_crsr);
  ib_tpl_t key_l5_c = ib_sec_search_tuple_create(index_crsr);
  ib_tpl_t key_l5_n = ib_clust_search_tuple_create(crsr_l5_n);
  size_t size = pos_custkey_array.size();
  int res;
  // TODO: first fetches all l5_pos for custkeys
  for (int i = 0; i < size; ++i) {
    // custkey
    ib_tuple_write_u32(key_l5_c, 0, pos_custkey_array[i].val);
    err = ib_cursor_moveto(index_crsr, key_l5_c, IB_CUR_GE, &res);
    if (err != DB_SUCCESS) {
      std::cout << "failed moving cursor for l5_c" << std::endl;
    }
    err = ib_cursor_read_row(index_crsr, tpl_l5_c);
    if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
    }
    ib_u32_t l5_pos;
    ib_tuple_read_u32(tpl_l5_c, 1, &l5_pos);
    ib_tuple_write_u32(key_l5_n, 0, l5_pos);
    err = ib_cursor_moveto(crsr_l5_n, key_l5_n, IB_CUR_GE, &res);
    if (err != DB_SUCCESS) {
      std::cout << "failed moving cursor for l5_c" << std::endl;
    }
    if (res == 0) {
      err = ib_cursor_read_row(crsr_l5_n, tpl_l5_n);
      ib_u32_t c_nationkey;
      ib_tuple_read_u32(tpl_l5_n, 1, &c_nationkey);
      if (c_nationkey == pred_c_nationkey) {
        pos_array.push_back(pos_custkey_array[i].pos);
      }
    }
    tpl_l5_c = ib_tuple_clear(tpl_l5_c);
  }
  ib_tuple_delete(key_l5_c);
  ib_tuple_delete(key_l5_n);
  ib_cursor_close(index_crsr);
  return pos_array;
}

std::vector<uint32_t>
apply_l5_predicate_scan(std::vector<im_val_t> pos_custkey_array)
{
  ib_err_t err;
  std::vector<uint32_t> pos_array;
  ib_crsr_t index_crsr;
  err = ib_cursor_open_index_using_name(crsr_l5_c, "SECONDARY_KEY", &index_crsr);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << std::endl; }
  ib_cursor_set_cluster_access(index_crsr);
  ib_tpl_t key_l5_c = ib_sec_search_tuple_create(index_crsr);
  ib_tpl_t key_l5_n = ib_clust_search_tuple_create(crsr_l5_n);
  size_t size = pos_custkey_array.size();
  int res;
  // TODO: first fetches all l5_pos for custkeys
  std::vector<uint32_t> l5_pos_array;
  for (int i = 0; i < size; ++i) {
    // custkey
    ib_tuple_write_u32(key_l5_c, 0, pos_custkey_array[i].val);
    err = ib_cursor_moveto(index_crsr, key_l5_c, IB_CUR_GE, &res);
    if (err != DB_SUCCESS) {
      std::cout << "failed moving cursor for l5_c" << std::endl;
    }
    err = ib_cursor_read_row(index_crsr, tpl_l5_c);
    if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
    }
    ib_u32_t l5_pos;
    ib_tuple_read_u32(tpl_l5_c, 1, &l5_pos);
    l5_pos_array.push_back(l5_pos);
    tpl_l5_c = ib_tuple_clear(tpl_l5_c);
  }
  for (int i = 0; i < size; ++i) {
    ib_tuple_write_u32(key_l5_n, 0, l5_pos_array[i]);
    err = ib_cursor_moveto(crsr_l5_n, key_l5_n, IB_CUR_GE, &res);
    if (err != DB_SUCCESS) {
      std::cout << "failed moving cursor for l5_c" << std::endl;
    }
    if (res == 0) {
      err = ib_cursor_read_row(crsr_l5_n, tpl_l5_n);
      ib_u32_t c_nationkey;
      ib_tuple_read_u32(tpl_l5_n, 1, &c_nationkey);
      if (c_nationkey == pred_c_nationkey) {
        pos_array.push_back(pos_custkey_array[i].pos);
      }
    }
    tpl_l5_n = ib_tuple_clear(tpl_l5_n);
  }
  ib_tuple_delete(key_l5_c);
  ib_tuple_delete(key_l5_n);
  ib_cursor_close(index_crsr);
  return pos_array;
}

std::vector<im_val_t>
get_l3_custkeys_from_sequences(int pos, int freq)
{
  ib_err_t err;
  std::vector<im_val_t> pos_custkey_array;
  ib_tpl_t key = ib_clust_search_tuple_create(crsr_l3_c);
  ib_tuple_write_u32(key, 0, pos);
  int res;
  static bool moved = false;
  if (!moved) {
    err = ib_cursor_moveto(crsr_l3_c, key, IB_CUR_GE, &res);
    ib_tuple_delete(key);
  }
  for (uint32_t i = pos; i < pos + freq; ++i) {
    err = ib_cursor_read_row(crsr_l3_c, tpl_l3_c);

    ib_u32_t custkey;
    ib_tuple_read_u32(tpl_l3_c, 1, &custkey);
    im_val_t im_val = {i, (uint32_t) custkey};
    pos_custkey_array.push_back(im_val);

    err = ib_cursor_next(crsr_l3_c);
    tpl_l3_c = ib_tuple_clear(tpl_l3_c);
  }
  return pos_custkey_array;
}

bool init_db(void)
{
  ib_err_t err;
  /* Initialize the memory sub-system. */
  ib_init();

  /* Call the ib_cfg_*() functions to setup the directory etc. */
  err = ib_cfg_set_bool_off("adaptive_hash_index");
  err = ib_cfg_set_int("additional_mem_pool_size", 16*1024*1024);
  err = ib_cfg_set_int("buffer_pool_size", 10*1024*1024);
  err = ib_cfg_set_int("flush_log_at_trx_commit", 2);
  err = ib_cfg_set_int("log_buffer_size", 8*1024*1024);
  err = ib_cfg_set_int("log_file_size", 256*1024*1024);
  err = ib_cfg_set_text("data_home_dir", "/data/");
  err = ib_cfg_set_text("log_group_home_dir", "/data/");
  err = ib_cfg_set_text("flush_method", "O_DIRECT");

  /* Create system files if this is the first time
   * or do recovery if starting an existing instance. */
  err = ib_startup("barracuda");
  /* File format "barracuda" supports all
   * the currently available table formats. */

  if (err == DB_SUCCESS) {
    printf("InnoDB started!\n");
  } else {
    printf("Error starting up InnoDB: %s\n", ib_strerror(err));
    return false;
  }
  return true;
}

bool fin_db(void)
{
  ib_err_t err = ib_shutdown(IB_SHUTDOWN_NORMAL);
  if (err == DB_SUCCESS) {
      printf("InnoDB shutdown succeed!\n");
  } else {
      printf("InnoDB shutdown failed: err %s\n", ib_strerror(err));
  }
}
