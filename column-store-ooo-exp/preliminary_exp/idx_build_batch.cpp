#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include <db_cxx.h>
#include "column.h"

template<typename T> bool bulk_put(Db *idx, T k, off_t v, bool last=false);

int main(int argc, char *argv[]) {

  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " idxlist idxname key_type(TINYINT|INT|BIGINT|DOUBLE)" << std::endl;
    exit(1);
  }

  char *idxlist = argv[1];
  char *idxname = argv[2];
  char *key_type = argv[3];
  cstore::col_t col_type;

  Db *idx_ = new Db(NULL, (u_int32_t) 0); 
  idx_->set_cachesize(2, 0, 1); 
  //idx_->set_bt_compare(cstore::compare_int32);
  if (strcmp(key_type, "TINYINT") == 0) {
    col_type = cstore::TINYINT;
    idx_->set_bt_compare(cstore::compare_int8);
  } else if (strcmp(key_type, "INT") == 0) {
    col_type = cstore::INT;
    idx_->set_bt_compare(cstore::compare_int32);
  } else if (strcmp(key_type, "BIGINT") == 0) {
    col_type = cstore::BIGINT;
    idx_->set_bt_compare(cstore::compare_int64);
  } else if (strcmp(key_type, "DOUBLE") == 0) {
    col_type = cstore::DOUBLE;
    idx_->set_bt_compare(cstore::compare_double);
  } else {
    std::cerr << "Usage: " << argv[0] << " idxlist idxname key_type(TINYINT|INT|BIGINT|DOUBLE)" << std::endl;
    exit(1);
  }
  idx_->set_flags(DB_DUP);

  u_int32_t oFlags = DB_CREATE;
  idx_->open(NULL, idxname, NULL, DB_BTREE, oFlags, 0);

  std::ifstream ifs(idxlist);
  if (!ifs) {
    std::cerr << "can't open " << idxlist << std::endl;
    exit(1);
  }

  std::string line;
  off_t v = 0;
  int i = 0;
  while (getline(ifs, line)) {
    std::vector<std::string> attrs;
    boost::split(attrs, line, boost::is_any_of(" "));
    if (attrs.size() == 2) {
      v = atoll(attrs.at(1).c_str());
    }
    switch (col_type) {
      case cstore::TINYINT:
        {
          int8_t k = atoll(attrs.at(0).c_str());
          bulk_put(idx_, k, v);
          break;
        }
      case cstore::INT:
        {
          int32_t k = atoll(attrs.at(0).c_str());
          bulk_put(idx_, k, v);
          break;
        }
      case cstore::BIGINT:
        {
          int64_t k = atoll(attrs.at(0).c_str());
          bulk_put(idx_, k, v);
          break;
        }
      case cstore::DOUBLE:
        {
          double k = atof(attrs.at(0).c_str());
          bulk_put(idx_, k, v);
          break;
        }
    }
    if (++i % 100000 == 0) {
      std::cout << "indexed " << i << std::endl;
    }
    ++v;
  }
  bulk_put(idx_, 0, 0, true);

  idx_->sync(0);
  idx_->close(0);

  return 0;
}

template<typename T>
bool bulk_put(Db *idx, T k, off_t v, bool last=false)
{
  static int num_entries = 0;
  static Dbt mkey, mdata;
  static char *keybuff = new char[1024*10240];
  static char *databuff = new char[1024*10240];
  static DbMultipleDataBuilder *keybuilder;
  static DbMultipleDataBuilder *databuilder;

  if (!last) {
    if (num_entries == 0) {
      memset(&mkey, 0, sizeof(Dbt)); 
      memset(&mdata, 0, sizeof(Dbt));
      memset(keybuff, 0, 1024*10240);
      mkey.set_ulen(1024*10240);
      mkey.set_data(keybuff);
      mkey.set_flags(DB_DBT_USERMEM | DB_DBT_BULK);

      memset(databuff, 0, 1024*10240);
      mdata.set_ulen(1024*10240);
      mdata.set_data(databuff);
      mdata.set_flags(DB_DBT_USERMEM | DB_DBT_BULK);

      delete(keybuilder);
      delete(databuilder);
      keybuilder = new DbMultipleDataBuilder(mkey);
      databuilder = new DbMultipleDataBuilder(mdata);
    }

    keybuilder->append(&k, sizeof(k));
    databuilder->append(&v, sizeof(off_t));
    num_entries++;
  }

  if (num_entries == 10000 || (last && num_entries > 0)) {
    int ret;
    if ((ret = idx->put(NULL, &mkey, &mdata, DB_MULTIPLE)) != 0) {
      idx->err(ret, "Db::put");
      throw DbException(ret);
    }
    num_entries = 0;
  }

  return true;
}
