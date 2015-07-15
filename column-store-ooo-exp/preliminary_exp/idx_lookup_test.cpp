#include <db_cxx.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "column.h"

int main(int argc, char *argv[]) {

  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " idx_name column_type(TINYINT|INT|BIGINT|CHAR|CHAR4|CHAR8) val" << std::endl;
    exit(1);
  }
  std::string idx_name(argv[1]);
  char *col_t = argv[2];
  char *val = argv[3];
  char v[8];
  memset(v, 0, 8);
  char *vp = v;

  Db *idx = new Db(NULL, (u_int32_t) 0); 

  cstore::col_t col_type;
  size_t col_size;

  if (strcmp(col_t, "TINYINT") == 0) {
    col_type = cstore::TINYINT;
    col_size = 1;
    int8_t tmp = atoi(val);
    memcpy(vp, &tmp, col_size);
    idx->set_bt_compare(cstore::compare_int8);
  } else if (strcmp(col_t, "INT") == 0) {
    col_type = cstore::INT;
    col_size = 4;
    int32_t tmp = atoi(val);
    memcpy(vp, &tmp, col_size);
    idx->set_bt_compare(cstore::compare_int32);
  } else if (strcmp(col_t, "BIGINT") == 0) {
    col_type = cstore::BIGINT;
    col_size = 8;
    int64_t tmp = atoll(val);
    memcpy(vp, &tmp, col_size);
    idx->set_bt_compare(cstore::compare_int64);
  } else if (strcmp(col_t, "CHAR") == 0) {
    col_type = cstore::CHAR;
    col_size = 1;
    vp = val;
  } else if (strcmp(col_t, "CHAR4") == 0) {
    col_type = cstore::CHAR4;
    col_size = 4;
    vp = val;
  } else if (strcmp(col_t, "CHAR8") == 0) {
    col_type = cstore::CHAR8;
    col_size = 8;
    vp = val;
  }

  //db->set_cachesize(1, 512*1024*1024, 1); 
  u_int32_t oFlags = DB_RDONLY;
  idx->open(NULL, idx_name.c_str(), NULL, DB_BTREE, oFlags, 0);

  Dbt idx_k;
  Dbt idx_v;
  memset(&idx_k, 0, sizeof(Dbt)); 
  memset(&idx_v, 0, sizeof(Dbt)); 
  idx_k.set_size(col_size);
  idx_v.set_size(sizeof(off_t));

  std::cout << "[" << idx_name << "]" << std::endl;
  std::string::size_type p = idx_name.find(".idx", 0);
  if (p == std::string::npos) {
    std::cerr << "idx_name and col_name mapping is wrong." << std::endl;
    exit(1);
  }
  std::cout << "p = " << p << std::endl;
  std::string col_name = idx_name.substr(0, p);
  std::cout << "[" << col_name << "]" << std::endl;

  cstore::Column *c = new cstore::Column();
  if (!c->open(col_name.c_str(), COL_RDONLY)) {
    std::cerr << "opening " << col_name << " failed." << std::endl;
    exit(1);
  }

  Dbc *cursorp;
  //cstore::rle_triple_tuple_t t;
  try {
    idx->cursor(NULL, &cursorp, 0);
    off_t pos;
    idx_k.set_data(vp);
    idx_v.set_data(&pos);
    idx_v.set_ulen(sizeof(off_t));
    idx_v.set_flags(DB_DBT_USERMEM);
    int ret;
    ret = cursorp->get(&idx_k, &idx_v, DB_SET);
    do {
      if (ret != 0) {
        /*
        std::cout << "ret = " << ret << std::endl;
        if (ret == DB_NOTFOUND) {
          std::cout << "not found" << std::endl;
        }
        */
        break;
      }
      std::cout << "pos: " << pos << std::endl;
      /*
      c->lookup_bypos(pos, &t);
      std::cout << "start: " << t.start << std::endl;
      std::cout << "length: " << t.length << std::endl;
      */
      ret = cursorp->get(&idx_k, &idx_v, DB_NEXT_DUP);
    } while (true);

  } catch(DbException &e) {
    idx->err(e.get_errno(), "Error!");
  } catch(std::exception &e) {
    idx->errx("Error! %s", e.what());
  }
  cursorp->close();
  idx->close(0);
  c->close();

}
