#include <haildb.h>
#include <assert.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

bool init_db(void);
bool fin_db(void);
bool create_db(char *db_name);
bool create_schema(char *table_name);

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
  if (!create_db(db_name)) {
    std::cerr << "create_db failed" << std::endl;
    exit(1);
  }

  sprintf(full_table_name, "%s/%s", db_name, table_name);
  if (!create_schema(full_table_name)) {
    std::cerr << "create_schema failed" << std::endl;
    exit(1);
  }

  ib_crsr_t ib_crsr;

  std::string line;
  int i = 0;
  ib_trx_t ib_trx = ib_trx_begin(IB_TRX_REPEATABLE_READ);
  ib_cursor_open_table(full_table_name, ib_trx, &ib_crsr);
  while (getline(std::cin, line)) {
    std::vector<std::string> strs;
    boost::split(strs, line, boost::is_any_of(" ") );
    ib_tpl_t tpl = ib_clust_read_tuple_create(ib_crsr);
    ib_tuple_write_u32(tpl, 0, atoi(strs[0].c_str()));
    ib_tuple_write_u32(tpl, 1, atoi(strs[1].c_str()));
    ib_err_t err = ib_cursor_insert_row(ib_crsr, tpl);
    if (err != DB_SUCCESS) {
      std::cerr << "insert_row failed" << std::endl;
    }
    ib_tuple_delete(tpl);
    if (++i % 10000 == 0) {
      std::cout << i << std::endl;
      ib_cursor_close(ib_crsr); 
      ib_trx_commit(ib_trx);
      ib_trx = ib_trx_begin(IB_TRX_REPEATABLE_READ);
      ib_cursor_open_table(full_table_name, ib_trx, &ib_crsr);
    }
  }
  ib_cursor_close(ib_crsr); 
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

bool create_db(char *db_name)
{
  ib_bool_t res = ib_database_create(db_name);
  if (res == IB_FALSE) {
    return false;
  }
  return true;
}

bool create_schema(char *table_name)
{
  ib_trx_t ib_trx;
  ib_id_t table_id = 0;
  ib_tbl_sch_t ib_tbl_sch = NULL;
  ib_idx_sch_t ib_idx_sch = NULL;
  char full_table_name[256];
  /* Pass a table page size of 0, ie., use default page size. */
  ib_table_schema_create(table_name, &ib_tbl_sch, IB_TBL_COMPACT, 0);

  /* The fifth argument is currently not used. */
  ib_table_schema_add_col(ib_tbl_sch, "k", IB_INT, IB_COL_UNSIGNED, 0, 4);
  ib_table_schema_add_col(ib_tbl_sch, "v1", IB_INT, IB_COL_UNSIGNED, 0, 4);

  /* Index schema handle is "owned" by the table schema handle in this
   * case and will be deleted when the table schema handle is deleted. */
  ib_table_schema_add_index(ib_tbl_sch, "PRIMARY_KEY", &ib_idx_sch);

  /* Set prefix length to 0. */
  ib_index_schema_add_col(ib_idx_sch, "k", 0);
  ib_index_schema_add_col(ib_idx_sch, "v1", 0);
  ib_index_schema_set_clustered(ib_idx_sch);

  /* Create the transaction that will cover data dictionary update. */
  ib_trx = ib_trx_begin(IB_TRX_REPEATABLE_READ);

  /* Lock the data dictionary in exclusive mode */
  ib_schema_lock_exclusive(ib_trx);

  /* Create the actual table from the schema. The table id of the new
   * table will be returned in table_id. */
  ib_table_create(ib_trx, ib_tbl_sch, &table_id);

  /* Commit the transaction */
  ib_trx_commit(ib_trx);

  ib_table_schema_delete(ib_tbl_sch);

  return true;
}
