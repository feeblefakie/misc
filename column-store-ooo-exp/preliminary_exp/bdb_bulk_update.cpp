#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include <db_cxx.h>
#include "column.h"

int main(void)
{
  Db *dbp = new Db(NULL, (u_int32_t) 0); 
  u_int32_t oFlags = DB_CREATE;
  dbp->set_flags(DB_DUP);
  dbp->open(NULL, "tmp.db", NULL, DB_BTREE, oFlags, 0);

  static Dbt mkey, mdata;
  static char keybuff[4096];
  memset(keybuff, 0, 4096);
  mkey.set_ulen(4096 * sizeof(char));
  //mkey.set_size(4096 * sizeof(char));
  mkey.set_data(&keybuff);
  //mkey.set_flags(DB_DBT_USERMEM);
  mkey.set_flags(DB_DBT_USERMEM | DB_DBT_BULK);

  static char databuff[4096];
  memset(databuff, 0, 4096);
  mdata.set_ulen(4096 * sizeof(char));
  mdata.set_data(&databuff);
  //mdata.set_flags(DB_DBT_USERMEM);
  mdata.set_flags(DB_DBT_USERMEM | DB_DBT_BULK);

  static DbMultipleDataBuilder *keybuilder;
  keybuilder = new DbMultipleDataBuilder(mkey);
  static DbMultipleDataBuilder *databuilder;
  databuilder = new DbMultipleDataBuilder(mdata);

  for (int i = 0; i < 100; i++) {
    keybuilder->append(&i, 4);
    databuilder->append(&i, 4);
  }

  int ret;
  if ((ret = dbp->put(NULL, &mkey, &mdata, DB_MULTIPLE)) != 0) {
    dbp->err(ret, "Db::put");
    throw DbException(ret);
  }

  dbp->sync(0);
  dbp->close(0);


  return 0;
}
