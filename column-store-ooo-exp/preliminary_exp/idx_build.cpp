#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include <db_cxx.h>

int compare_int64(Db *dbp, const Dbt *a, const Dbt *b);

int main(int argc, char *argv[]) {

  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " idxlist idxname" << std::endl;
    exit(1);
  }

  char *idxlist = argv[1];
  char *idxname = argv[2];

  Db *idx_ = new Db(NULL, (u_int32_t) 0); 
  idx_->set_cachesize(1, 512*1024*1024, 1); 
  idx_->set_bt_compare(compare_int64);
  idx_->set_flags(DB_DUP);

  u_int32_t oFlags = DB_CREATE;
  idx_->open(NULL, idxname, NULL, DB_BTREE, oFlags, 0);

  std::ifstream ifs(idxlist);
  if (!ifs) {
    std::cerr << "can't open " << idxlist << std::endl;
    exit(1);
  }

  Dbt idx_k_;
  Dbt idx_v_;
  std::string line;
  while (getline(ifs, line)) {
    std::vector<std::string> attrs;
    boost::split(attrs, line, boost::is_any_of(" "));
    int64_t k = atoll(attrs.at(0).c_str());
    off_t v = atoll(attrs.at(1).c_str());

    memset(&idx_k_, 0, sizeof(Dbt)); 
    memset(&idx_v_, 0, sizeof(Dbt)); 
    idx_k_.set_size(sizeof(int64_t));
    idx_v_.set_size(sizeof(off_t));
    idx_k_.set_data(&k);
    idx_v_.set_data(&v);
    int status = idx_->put(NULL, &idx_k_, &idx_v_, 0);
    if (status != 0) {
      idx_->err(status, "Put failed for some reason.");
    }
  }

  idx_->sync(0);
  idx_->close(0);

  return 0;
}

int compare_int64(Db *dbp, const Dbt *a, const Dbt *b)
{
  int64_t i = *((int64_t *) a->get_data());
  int64_t j = *((int64_t *) b->get_data());

  if (i > j)
    return 1;
  if (i < j)
    return -1;
  return 0;
}
