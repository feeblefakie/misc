#include <haildb.h>
#include <assert.h>
#include <iostream>
#include <stdlib.h>

bool init_db(void);
bool fin_db(void);
bool create_db(char *db_name);
bool create_schema(char *table_name);
void scan(char *table_name);
void scan_from(char *table_name, int offset);

int main(int argc, char *argv[])
{
  if (argc != 2) {
    exit(1);
  }
  char db_name[32] = "/data/cstore";
  char *table_name = argv[1];
  char full_table_name[32];

  if (!init_db()) {
    exit(1);
  }

  sprintf(full_table_name, "%s/%s", db_name, table_name);

  //scan(full_table_name);
  scan_from(full_table_name, 1000);

  /*
  ib_crsr_t ib_crsr;
  std::string line;
  int i = 0;
  ib_trx_t ib_trx = ib_trx_begin(IB_TRX_REPEATABLE_READ);
  ib_cursor_open_table(full_table_name, ib_trx, &ib_crsr);
  while (getline(std::cin, line)) {
    int num = atoi(line.c_str());
    int res;
    ib_tpl_t key = ib_clust_search_tuple_create(ib_crsr);
    ib_tuple_write_u32(key, 0, num);
    ib_err_t err = ib_cursor_moveto(ib_crsr, key, IB_CUR_GE, &res);
    ib_tuple_delete(key);
    if (err == DB_SUCCESS) {
      //std::cout << "success" << std::endl;
    } else {
      std::cout << "failed" << std::endl;
      ib_cursor_close(ib_crsr); 
      continue;
    }
    if (res == 0) { // exact match
      ib_tpl_t tpl = ib_clust_read_tuple_create(ib_crsr);
      err = ib_cursor_read_row(ib_crsr, tpl);
      if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
        std::cerr << "read_row failed" << std::endl;
      }
      ib_u32_t v;
      ib_tuple_read_u32(tpl, 1, &v);

      //std::cout << "got: " << v << std::endl;
      ib_tuple_delete(tpl);
    } else {
      std::cerr << "no match" << std::endl;
    }
    if (++i % 10000 == 0) {
      std::cout << i << std::endl;
    }
  }
  ib_cursor_close(ib_crsr); 
  ib_trx_commit(ib_trx);
  */
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
  ib_table_schema_add_col(ib_tbl_sch, "sk", IB_INT, IB_COL_UNSIGNED, 0, 4);
  ib_table_schema_add_col(ib_tbl_sch, "quantity", IB_INT, IB_COL_UNSIGNED, 0, 4);

  /* Index schema handle is "owned" by the table schema handle in this
   * case and will be deleted when the table schema handle is deleted. */
  ib_table_schema_add_index(ib_tbl_sch, "PRIMARY_KEY", &ib_idx_sch);

  /* Set prefix length to 0. */
  ib_index_schema_add_col(ib_idx_sch, "sk", 0);
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

void scan(char *table_name)
{
  std::cout << "scan start" << std::endl;
  ib_err_t  err;
  ib_crsr_t crsr;

  ib_trx_t ib_trx = ib_trx_begin(IB_TRX_REPEATABLE_READ);
  ib_cursor_open_table(table_name, ib_trx, &crsr);

  ib_tpl_t tpl = ib_clust_read_tuple_create(crsr);

  /* Scan the entire table and print all rows. */
  err = ib_cursor_first(crsr);

  while (err == DB_SUCCESS) {
    err = ib_cursor_read_row(crsr, tpl);

    /* Possible handle locking and timeout errors too
     *  in multi-threaded applications. */
    if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
      break;
    }
    ib_u32_t v;
    ib_tuple_read_u32(tpl, 1, &v);
    std::cout << "got: " << v << std::endl;

    err = ib_cursor_next(crsr);

    /* Possible handle locking and timeout errors too
     *  in multi-threaded applications. */
    if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
      break;
    }
    tpl = ib_tuple_clear(tpl);
  }
  ib_tuple_delete(tpl);
    ib_cursor_close(crsr); 
    ib_trx_commit(ib_trx);
  std::cout << "scan end" << std::endl;
}

void scan_from(char *table_name, int offset)
{
  std::cout << "scan start" << std::endl;
  ib_err_t err;
  ib_crsr_t crsr;

  ib_trx_t ib_trx = ib_trx_begin(IB_TRX_REPEATABLE_READ);
  ib_cursor_open_table(table_name, ib_trx, &crsr);

  ib_tpl_t tpl = ib_clust_read_tuple_create(crsr);

  /* Scan the entire table and print all rows. */
  //err = ib_cursor_first(crsr);

  ib_tpl_t key = ib_clust_search_tuple_create(crsr);
  ib_tuple_write_u32(key, 0, offset);
  int res;
  err = ib_cursor_moveto(crsr, key, IB_CUR_GE, &res);
  ib_tuple_delete(key);
  if (err == DB_SUCCESS) {
    //std::cout << "success" << std::endl;
  } else {
    std::cout << "failed" << std::endl;
    ib_cursor_close(crsr); 
    return;
  }
  if (res != 0) { // exact match
    std::cout << "no match. returning." << std::endl;
    return;
  }

  while (err == DB_SUCCESS) {
    err = ib_cursor_read_row(crsr, tpl);

    /* Possible handle locking and timeout errors too
     *  in multi-threaded applications. */
    if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
      break;
    }
    ib_u32_t v;
    ib_tuple_read_u32(tpl, 1, &v);
    std::cout << "got: " << v << std::endl;

    err = ib_cursor_next(crsr);

    /* Possible handle locking and timeout errors too
    *  in multi-threaded applications. */
    if (err == DB_RECORD_NOT_FOUND || err == DB_END_OF_INDEX) {
      break;
    }
    tpl = ib_tuple_clear(tpl);
  }
  ib_tuple_delete(tpl);
  ib_cursor_close(crsr); 
  ib_trx_commit(ib_trx);
  std::cout << "scan end" << std::endl;
}
