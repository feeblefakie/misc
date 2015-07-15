#include <haildb.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <assert.h>

char db_name[32] = "/data/cstore";

bool init_db(void);
bool fin_db(void);
int main(int argc, char *argv[])
{
  if (!init_db()) { exit(1); }

  ib_crsr_t crsr, index_crsr;
  ib_tpl_t tpl;
  ib_err_t err;
  char *l6_orderkey_rle = "l5_customer_key_rle";
  char cstore_l6_orderkey_rle[64];
  sprintf(cstore_l6_orderkey_rle, "%s/%s", db_name, l6_orderkey_rle);
  ib_trx_t ib_trx = ib_trx_begin(IB_TRX_REPEATABLE_READ);
  err = ib_cursor_open_table(cstore_l6_orderkey_rle, ib_trx, &crsr);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << " " << cstore_l6_orderkey_rle << std::endl; }
  err = ib_cursor_open_index_using_name(crsr, "SECONDARY_KEY", &index_crsr);
  if (err != DB_SUCCESS) { std::cout << ib_strerror(err) << std::endl; }

  ib_cursor_set_cluster_access(index_crsr);
  ib_tpl_t key = ib_sec_search_tuple_create(index_crsr);
  ib_tuple_write_u32(key, 0, 330575);
  int res;
  err = ib_cursor_moveto(index_crsr, key, IB_CUR_GE, &res);
  std::cout << "res: " << res << std::endl;
  assert(err == DB_SUCCESS || err == DB_END_OF_INDEX || err == DB_RECORD_NOT_FOUND);
  tpl = ib_clust_read_tuple_create(crsr);
  while (err == DB_SUCCESS) {
    err = ib_cursor_read_row(index_crsr, tpl);
    if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
      std::cerr << "not found" << std::endl;
    }
    ib_u32_t orderkey, pos, freq;
    ib_tuple_read_u32(tpl, 0, &orderkey);
    ib_tuple_read_u32(tpl, 1, &pos);
    ib_tuple_read_u32(tpl, 2, &freq);
    std::cout << "orderkey: " << orderkey << ", pos: " << pos << ", freq: " << freq << std::endl;
    err = ib_cursor_next(index_crsr);
    tpl = ib_tuple_clear(tpl);
    break;
  }
  ib_cursor_close(index_crsr); 
  ib_cursor_close(crsr); 
  ib_trx_commit(ib_trx);

  fin_db();

  return 0;
}

bool init_db(void)
{
  ib_err_t err;
  /* Initialize the memory sub-system. */
  ib_init();

  /* Call the ib_cfg_*() functions to setup the directory etc. */
  err = ib_cfg_set_bool_off("adaptive_hash_index");
  err = ib_cfg_set_int("additional_mem_pool_size", 16*1024*1024);
  err = ib_cfg_set_int("buffer_pool_size", 1024*1024*1024);
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
