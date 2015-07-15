#include <haildb.h>
#include <assert.h>
#include <iostream>
#include <stdlib.h>
#include <string>

bool init_db(void);
bool fin_db(void);

int main(int argc, char *argv[])
{
  if (argc != 2) {
    exit(1);
  }
  char db_name[32] = "cstore";
  char *table_name = argv[1];
  char full_table_name[32];
  if (!init_db()) {
    exit(1);
  }

  sprintf(full_table_name, "%s/%s", db_name, table_name);

  ib_trx_t ib_trx = ib_trx_begin(IB_TRX_REPEATABLE_READ);
  ib_schema_lock_exclusive(ib_trx);
  ib_err_t err = ib_table_drop(ib_trx, full_table_name);
  if (err != DB_SUCCESS) {
    std::cout << "error: " << ib_strerror(err) << std::endl;
  }
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
  err = ib_cfg_set_int("buffer_pool_size", 2*1024*1024*1024);
  err = ib_cfg_set_int("flush_log_at_trx_commit", 2);
  err = ib_cfg_set_int("log_buffer_size", 8*1024*1024);
  err = ib_cfg_set_int("log_file_size", 256*1024*1024);
  err = ib_cfg_set_text("data_home_dir", "/data/");
  err = ib_cfg_set_text("log_group_home_dir", "/data/");
  err = ib_cfg_set_bool_on("file_per_table");
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

