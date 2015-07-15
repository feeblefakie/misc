#include <iostream>
#include <fstream>
#include <sstream>
#include <deque>
#include <vector>
#include <linux/unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <unordered_map>
#include "ops.h"
#include "query_util.h"

struct task {
  uint32_t table_id;
  int32_t part_id;
  uint32_t column_id;
  off_t pos;
  //char *val; // for join
  int64_t val; // for join
  std::vector<off_t> lm_poses;
  cstore::col_t col_type;
  int (*iofuncp)(int tid, task *t);
  int (*opfuncp)(int tid, task *t);
};

struct targ {
  int tid;
}; 

struct sarg {
  int part_id;
  int table_id;
}; 

struct table_func {
  void (*start)(int table_id);
  std::vector<std::string> (*prepare)(char *def, std::vector<char *> pred, std::vector<char *> comp,
                                      std::vector< std::vector<cstore::DS4 *> > &dsv,
                                      std::vector<void *> &preds,
                                      std::vector< std::vector<Db *> > &idxes,
                       std::vector<cstore::col_t> &pred_col_types);
};

std::deque<task *> q1;
std::deque<task *> q;

static double gettimeofday_sec();
void *task_processor(void *p);

int nlj_em_cio1(int tid, task *t);
int nlj_em_cop1(int tid, task *t);
int nlj_em_cio2(int tid, task *t);
int nlj_em_cop2(int tid, task *t);
int nlj_lm_cio1(int tid, task *t);
int nlj_lm_cio2(int tid, task *t);
int hj_em_cio1(int tid, task *t);
int hj_em_cop1(int tid, task *t);
int hj_em_cio2(int tid, task *t);
int hj_em_cop2(int tid, task *t);
void nlj_em_start1(int table_id);
void nlj_em_start2(int table_id);
void scan_em_start1(int table_id);
void *scan_em_start1_part(void *p);
void scan_lm_start1(int table_id);
void *scan_lm_start1_part(void *p);

void hj_lm_start2(int table_id);
void *hj_lm_start2_part(void *p);

int opempty(int tid, task *t);
int iolookup(int tid, task *t);

std::vector<std::string> nlj_prepare_outer(char *table_def, std::vector<char *> table_preds, std::vector<char *> table_comps, 
                                           std::vector< std::vector<cstore::DS4 *> > &dsv,
                                           std::vector<void *> &preds,
                                           std::vector< std::vector<Db *> > &idxes,
                                           std::vector<cstore::col_t> &pred_col_types);
std::vector<std::string> hj_prepare_outer(char *table_def, std::vector<char *> table_preds, std::vector<char *> table_comps,
                                           std::vector< std::vector<cstore::DS4 *> > &dsv,
                                           std::vector<void *> &preds,
                                           std::vector< std::vector<Db *> > &idxes,
                                           std::vector<cstore::col_t> &pred_col_types);

table_func nlj_em[] = {
  {nlj_em_start1, nlj_prepare_outer}
};

table_func scan_em[] = {
  {scan_em_start1, hj_prepare_outer}
};

table_func scan_lm[] = {
  {scan_lm_start1, hj_prepare_outer}
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<void *> preds;
std::vector<cstore::col_t> pred_col_types;
std::vector< std::vector<Db *> > idxes;
std::vector< std::vector<cstore::DS2 *> > scanss;
std::vector< std::vector< std::vector<cstore::DS4 *> > > dsv;
//pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t h_mutex = PTHREAD_MUTEX_INITIALIZER;
uint64_t **produced;
uint64_t **done;
uint64_t num_columns[3] = {0, 0, 0};
int num_threads = 1;
int num_tables = 1;
int num_preds = 1;
bool is_lm = false;
bool hj_scanned[2] = {false, false};
bool hj_probables[2] = {false, false};
std::unordered_map<int64_t, bool> h1;
std::unordered_map<int64_t, bool> h2;
std::unordered_map<int64_t, bool> hs[2] = {h1, h2};

std::vector<off_t> lm_poses[3];
char *ooo;
int parallelism = 1; // for HJ

int main(int argc, char *argv[]) {

  if (argc < 8) {
    std::cerr << "Usage: " << argv[0] << " join_method(H|N) materialization(EM|LM) out-of-order(on|off) num_threads table_def num_preds pred1 comp1 [pred2 comp2]" << std::endl;
    exit(1);
  }
  char *join = argv[1];
  char *mat = argv[2];
  if (strncmp(mat, "LM", 2) == 0) {
    is_lm = true;
  }
  ooo = argv[3];
  num_threads = atoi(argv[4]);
  std::vector<char *> table_defs;
  table_defs.push_back(argv[5]);
  num_preds = atoi(argv[6]);
  std::vector<char *> table_preds;
  std::vector<char *> table_comps;
  int argn = 7;
  for (int i = 0; i < num_preds; ++i) {
    table_preds.push_back(argv[argn++]);
    table_comps.push_back(argv[argn++]);
  }
  if (argc > argn) {
    parallelism = atoi(argv[argn]);
    if (parallelism != 16) {
      std::cerr << "currently 1 or 16 is only supported." << std::endl;
      exit(1);
    }
  }
  std::cout << "parallelism: " << parallelism << std::endl;

  table_func *f = new table_func[num_tables];
  if (strncmp(join, "N", 1) == 0) {
    if (strncmp(mat, "EM", 2) == 0) {
      // NLJ-EM
      for (int i = 0; i < num_tables; ++i) {
        f[i] = nlj_em[i];
      }
    } else {
      // NLJ-LM
      for (int i = 0; i < num_tables; ++i) {
        f[i] = nlj_em[i];
      }
    }
  } else {
    if (strncmp(mat, "EM", 2) == 0) {
      f[0] = scan_em[0];
    } else {
      f[0] = scan_lm[0];
    }
  }

  std::vector< std::vector<cstore::DS4 *> > ds;
  std::vector<std::string> tcolumns = f[0].prepare(table_defs.at(0), table_preds, table_comps, ds, preds, idxes, pred_col_types);
  num_columns[0] = tcolumns.size();
  dsv.push_back(ds);

  produced = new uint64_t*[num_tables];
  done = new uint64_t*[num_tables];
  for (uint64_t i = 0; i < num_tables; i++) {
    produced[i] = new uint64_t[num_columns[i]];
    done[i] = new uint64_t[num_columns[i]];
  }

  for (uint64_t i = 0; i < num_tables; i++) {
    for (uint64_t j = 0; j < num_columns[i]; j++) {
      produced[i][j] = 0;
      done[i][j] = 0;
    }
  }

  // create lookup threads
  pthread_t readers[num_threads];
  targ args[num_threads];
  for (int i = 0; i < num_threads; ++i) {
    args[i].tid = i;
    if (pthread_create(&readers[i], NULL, task_processor, (void *) &args[i]) != 0) {
      perror("pthread_create");
      exit(1);
    }
  }

  double tstart = gettimeofday_sec();
  for (int i = 0; i < num_tables; ++i) {
    f[i].start(i);
  }
  // HJ-EM
  if (strncmp(mat, "EM", 2) == 0 && strncmp(join, "H", 1) == 0) {
    double tend = gettimeofday_sec();
    std::cerr << "etime: " << tend - tstart << std::endl;
    return 0;
  }
  // HJ-LM
  if (strncmp(mat, "LM", 2) == 0 && strncmp(join, "H", 1) == 0) {
    // late materialization
    int cnt = 0;
    std::cout << "Late materialization phase" << std::endl;
    double lmstart = gettimeofday_sec();
    /* NOTICE: lm accesses are issued everytime it needs to be (see. TODO2)
    for (int i = 0; i < num_tables; ++i) {
      int j = 3;
      if (i == 0) {
        j = 2;
      }
      int cnt2 = 0;
      for (; j < num_columns[i]; ++j) {
        // NOTICE: uses only right most poses
        for (int k = 0; k < lm_poses[num_tables-1].size(); ++k) {
          task *t = new task();
          t->table_id = i;
          t->column_id = j;
          //t->pos = lm_poses[i].at(k);
          t->pos = lm_poses[num_tables-1].at(k);
          t->iofuncp = iolookup;
          t->opfuncp = opempty;
          pthread_mutex_lock(&mutex);
          q.push_back(t);
          pthread_mutex_unlock(&mutex);
          ++cnt;
          ++cnt2;
        }
      }
      std::cout << "cnt2: " << cnt2 << std::endl;
    }
    */
    while (true) {
      pthread_mutex_lock(&mutex);
      // NOTICE: outstanding ones are ignored for now
      if (q.empty()) {
        break;
      }
      pthread_mutex_unlock(&mutex);
      usleep(100000);
    }
    double lmend = gettimeofday_sec();
    std::cerr << "cnt: " << cnt << std::endl;
    std::cerr << "etime(lm-phase): " << lmend - lmstart << std::endl;
    double tend = gettimeofday_sec();
    std::cerr << "etime: " << tend - tstart << std::endl;
    exit(0);
  }

  int i_done = 0;
  uint64_t j_done = 0;
  while (true) {
    bool is_done = true;
    for (int i = i_done; i < num_tables; i++) {
      for (uint64_t j = j_done; j < num_columns[i]; j++) {
        //pthread_mutex_lock(&counter_mutex);
        pthread_mutex_lock(&mutex);
        if (produced[i][j] != done[i][j]) {
          //pthread_mutex_unlock(&counter_mutex);
          pthread_mutex_unlock(&mutex);
          is_done = false;
          break;
        }
        //pthread_mutex_unlock(&counter_mutex);
        pthread_mutex_unlock(&mutex);
      }
    }
    if (is_done) {
      for (uint64_t i = 0; i < num_columns[0]; i++) {
        std::cout << "produced[0][" << i << "] = " + produced[0][i] << ", done[0][" << i << "] = " + done[0][i] << std::endl;
      }
      break;
    }
    usleep(100000);
  }
  double tend = gettimeofday_sec();
  std::cerr << "etime: " << tend - tstart << std::endl;

  return 0;
}

static double 
gettimeofday_sec()
{
  struct timeval tv; 
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void *task_processor(void *p)
{
  targ *arg = (targ *) p;
  static int i = 0;
  while (true) {
    // deque
    task *t;
    pthread_mutex_lock(&mutex);
    if (q.empty()) {
      if (q1.empty()) {
        pthread_mutex_unlock(&mutex);
        usleep(100000);
        continue;
      } else {
        t = q1.front();
        q1.pop_front();
      }
    } else {
      t = q.front();
      q.pop_front();
    }
    pthread_mutex_unlock(&mutex);

    t->iofuncp(arg->tid, t);
    //t->opfuncp(arg->tid, t);
    //pthread_mutex_lock(&counter_mutex);
    pthread_mutex_lock(&mutex);
    done[t->table_id][t->column_id]++;
    /*
    ++i;
    if (i % 1000 == 0) {
      std::cout << i << "processed" << std::endl;
    }
    */
    //pthread_mutex_unlock(&counter_mutex);
    pthread_mutex_unlock(&mutex);
    delete(t);
  }
  return NULL;
}

int hj_em_cio1(int tid, task *t)
{
  uint32_t table_id = t->table_id;
  uint32_t column_id = t->column_id;
  off_t pos = t->pos;
  //std::cout << "table: " << table_id << ", io for column: " << column_id << ", pos: " << pos << std::endl;
  //cstore::nocomp_tuple_t nocomp_tuple;
  cstore::rle_triple_tuple_t rle_tuple;
  cstore::nocomp_tuple_t *nocomp_tuple;
  bool passed;
  bool ret;

  cstore::DS4 *ds4 = dsv.at(table_id).at(column_id-1).at(tid);
  if (ds4->get_comp_type() == cstore::RLE) {
    ret = ds4->rle_op(pos, &rle_tuple, &passed);
    //std::cout << "rle_op" << std::endl;
  } else {
    ret = ds4->nocomp_op_new(pos, (void **) &nocomp_tuple, &passed);
    //std::cout << "nocomp_op" << std::endl;
  }
  if (!passed) { 
    return 0;
  }

  // building a hash table
  if (column_id == 2) {
    // NOTICE: hard-coded type
    int64_t val;
    if (ds4->get_comp_type() == cstore::RLE) {
      memcpy(&val, rle_tuple.val, 8);
    } else {
      memcpy(&val, nocomp_tuple->val, 8);
    }
    pthread_mutex_lock(&h_mutex);
    hs[0][val] = true;
    pthread_mutex_unlock(&h_mutex);
    //std::cout << "built: " << val << std::endl;
  }

  if (column_id == dsv.at(table_id).size()) {
    return 0;
  } 

  //pthread_mutex_lock(&counter_mutex);
  //pthread_mutex_unlock(&counter_mutex);
  task *t2;
  t2 = new task();
  t2->table_id = 0;
  t2->column_id = column_id+1;
  t2->pos = pos;
  t2->iofuncp = hj_em_cio1;
  t2->opfuncp = hj_em_cop1;

  pthread_mutex_lock(&mutex);
  produced[t2->table_id][t2->column_id]++;
  q.push_back(t2);
  pthread_mutex_unlock(&mutex);

  return 0;

}

int hj_em_cio2(int tid, task *t)
{
  uint32_t table_id = t->table_id;
  uint32_t column_id = t->column_id;
  off_t pos = t->pos;
  //std::cout << "table: " << table_id << ", io for column: " << column_id << ", pos: " << pos << std::endl;
  cstore::rle_triple_tuple_t rle_tuple;
  cstore::nocomp_tuple_t *nocomp_tuple;
  bool passed;
  bool ret;

  cstore::DS4 *ds4 = dsv.at(table_id).at(column_id-1).at(tid);
  if (ds4->get_comp_type() == cstore::RLE) {
    ret = ds4->rle_op(pos, &rle_tuple, &passed);
    //std::cout << "rle_op" << std::endl;
  } else {
    ret = ds4->nocomp_op_new(pos, (void **) &nocomp_tuple, &passed);
    //std::cout << "nocomp_op" << std::endl;
  }
  if (!passed) { 
    return 0;
  }

  // probing column
  if (column_id == 1) {
    // NOTICE: hard-coded type
    int64_t val;
    if (ds4->get_comp_type() == cstore::RLE) {
      memcpy(&val, rle_tuple.val, 8);
    } else {
      memcpy(&val, nocomp_tuple->val, 8);
    }
    //std::cout << "probing: " << val << std::endl;
    pthread_mutex_lock(&h_mutex);
    bool res = hs[table_id-1][val];
    pthread_mutex_unlock(&h_mutex);
    if (res) {
      std::cout << "probed: " << val << std::endl;
      std::cout << "table: " << table_id << ", io for column: " << column_id << ", pos: " << pos << std::endl;
    } else {
      return 0;
    }
  }

  // building a hash table
  if (column_id == 2 && table_id < num_tables-1) {
    // NOTICE: hard-coded type
    int64_t val;
    if (ds4->get_comp_type() == cstore::RLE) {
      memcpy(&val, rle_tuple.val, 8);
    } else {
      memcpy(&val, nocomp_tuple->val, 8);
    }
    pthread_mutex_lock(&h_mutex);
    hs[table_id][val] = true;
    pthread_mutex_unlock(&h_mutex);
    //std::cout << "built" << table_id << ": " << val << std::endl;
  }

  if (column_id == dsv.at(table_id).size()) {
    return 0;
  } 


  //pthread_mutex_lock(&counter_mutex);
  //pthread_mutex_unlock(&counter_mutex);
  task *t2;
  t2 = new task();
  t2->table_id = table_id;
  t2->column_id = column_id+1;
  t2->pos = pos;
  t2->iofuncp = hj_em_cio2;
  t2->opfuncp = hj_em_cop2;
  pthread_mutex_lock(&mutex);
  produced[t2->table_id][t2->column_id]++;
  q.push_back(t2);
  pthread_mutex_unlock(&mutex);

  return 0;

}

int hj_em_cop1(int tid, task *t) { }
int hj_em_cop2(int tid, task *t) { }

int nlj_em_cop1(int tid, task *t)
{
  // do nothing
}

int nlj_em_cop2(int tid, task *t)
{
  // do nothing
}

int nlj_em_cio1(int tid, task *t)
{
  uint32_t table_id = t->table_id;
  uint32_t column_id = t->column_id;
  off_t pos = t->pos;
  //std::cout << "table: " << table_id << ", io for column: " << column_id << ", pos: " << pos << std::endl;
  cstore::nocomp_tuple_t nocomp_tuple;
  cstore::rle_triple_tuple_t rle_tuple;
  bool passed;
  bool ret;

  cstore::DS4 *ds4 = dsv.at(table_id).at(column_id-1).at(tid);
  if (ds4->get_comp_type() == cstore::RLE) {
    ret = ds4->rle_op(pos, &rle_tuple, &passed);
    //std::cout << "rle_op" << std::endl;
  } else {
    ret = ds4->nocomp_op(pos, &nocomp_tuple, &passed);
    //std::cout << "nocomp_op" << std::endl;
  }
  if (!passed) { return 0; }

  task *t2 = NULL;
  if (column_id != dsv.at(table_id).size()) {
    t2 = new task();
    t2->table_id = 0;
    t2->column_id = column_id+1;
    t2->pos = pos;
    t2->iofuncp = nlj_em_cio1;
    t2->opfuncp = nlj_em_cop1;
  } else if (table_id + 1 < num_tables) {
    t2 = new task();
    t2->table_id = 1;
    t2->column_id = 0;
    t2->pos = pos;
    //t2->val = new char[8];
    if (ds4->get_comp_type() == cstore::RLE) {
      memcpy(&(t2->val), rle_tuple.val, 8);
    } else {
      memcpy(&(t2->val), nocomp_tuple.val, 8);
    }
    t2->col_type = ds4->get_col_type();
    t2->iofuncp = nlj_em_cio2;
    t2->opfuncp = nlj_em_cop2; // not used
  } else {
    return 0;
  }

  //pthread_mutex_lock(&counter_mutex);
  //pthread_mutex_unlock(&counter_mutex);
  pthread_mutex_lock(&mutex);
  produced[t2->table_id][t2->column_id]++;
  q.push_back(t2);
  pthread_mutex_unlock(&mutex);
  return 0;
}

int nlj_lm_cio1(int tid, task *t)
{
  uint32_t table_id = t->table_id;
  uint32_t column_id = t->column_id;
  off_t pos = t->pos;
  //std::cout << "table: " << table_id << ", io for column: " << column_id << ", pos: " << pos << std::endl;
  cstore::nocomp_tuple_t nocomp_tuple;
  cstore::rle_triple_tuple_t rle_tuple;
  bool passed;
  bool ret;

  cstore::DS4 *ds4 = dsv.at(table_id).at(column_id-1).at(tid);
  if (ds4->get_comp_type() == cstore::RLE) {
    ret = ds4->rle_op(pos, &rle_tuple, &passed);
    //std::cout << "rle_op" << std::endl;
  } else {
    ret = ds4->nocomp_op(pos, &nocomp_tuple, &passed);
    //std::cout << "nocomp_op" << std::endl;
  }
  if (!passed) { return 0; }

  task *t2 = NULL;
  if (column_id == dsv.at(table_id).size()) { return 0; }
  if (column_id == 1) {
    t2 = new task();
    t2->table_id = 1;
    t2->column_id = 0;
    t2->pos = pos;
    //t2->val = new char[8];
    if (ds4->get_comp_type() == cstore::RLE) {
      memcpy(&(t2->val), rle_tuple.val, 8);
    } else {
      memcpy(&(t2->val), nocomp_tuple.val, 8);
    }
    t2->col_type = ds4->get_col_type();
    t2->iofuncp = nlj_lm_cio2;
    t2->opfuncp = nlj_em_cop2; // not used
    t2->lm_poses.push_back(pos);
  } else {
    t2 = new task();
    t2->table_id = 0;
    t2->column_id = column_id+1;
    t2->pos = pos;
    t2->iofuncp = nlj_lm_cio1;
    t2->opfuncp = nlj_em_cop1;
    //t2->lm_poses = t->lm_poses;
  }

  //pthread_mutex_lock(&counter_mutex);
  //pthread_mutex_unlock(&counter_mutex);
  pthread_mutex_lock(&mutex);
  produced[t2->table_id][t2->column_id]++;
  q.push_back(t2);
  pthread_mutex_unlock(&mutex);
  return 0;
}

int nlj_em_cio2(int tid, task *t)
{
  uint32_t table_id = t->table_id;
  uint32_t column_id = t->column_id;
  //std::cout << "table: " << table_id << ", io for column: " << column_id << ", pos: " << t->pos << std::endl;

  if (column_id == 0) {
    // nlj inner table index lookup by a specified value
    Db *idx = idxes.at(table_id).at(tid);
    Dbt idx_k;
    Dbt idx_v;
    memset(&idx_k, 0, sizeof(Dbt)); 
    memset(&idx_v, 0, sizeof(Dbt)); 

    Dbc *cursorp;
    //cstore::rle_triple_tuple_t t;
    try {
      idx->cursor(NULL, &cursorp, 0);
      off_t pos;
      idx_k.set_data(&(t->val));
      idx_k.set_size(cstore::get_col_size(t->col_type));
      idx_v.set_data(&pos);
      idx_v.set_ulen(sizeof(off_t));
      idx_v.set_flags(DB_DBT_USERMEM);
      int ret = cursorp->get(&idx_k, &idx_v, DB_SET);
      do {
        // TODO: TO FIX
        if (ret != 0 || cstore::generic_compare(idx_k.get_data(), &(t->val), t->col_type) != 0) {
          if (ret == DB_NOTFOUND) {
            std::cout << "not found" << std::endl;
          }
          break;
        }
        task *t2 = new task();
        t2->table_id = table_id;
        t2->column_id = 1;
        t2->pos = pos;
        t2->iofuncp = nlj_em_cio2;
        t2->opfuncp = nlj_em_cop2;

        //pthread_mutex_lock(&counter_mutex);
        //pthread_mutex_unlock(&counter_mutex);

        pthread_mutex_lock(&mutex);
        produced[t2->table_id][t2->column_id]++;
        q.push_back(t2);
        pthread_mutex_unlock(&mutex);
        ret = cursorp->get(&idx_k, &idx_v, DB_NEXT);
      } while (true);

    } catch(DbException &e) {
      idx->err(e.get_errno(), "Error!");
    } catch(std::exception &e) {
      idx->errx("Error! %s", e.what());
    }
    cursorp->close();

  } else {
    // tuple reconstruction
    // 0 (w/ predicate), 1 (next JK), 2- (others)
    off_t pos = t->pos;
    cstore::nocomp_tuple_t nocomp_tuple;
    cstore::rle_triple_tuple_t rle_tuple;
    bool passed;
    bool ret;

    cstore::DS4 *ds4 = dsv.at(table_id).at(column_id-1).at(tid);
    if (ds4->get_comp_type() == cstore::RLE) {
      ret = ds4->rle_op(pos, &rle_tuple, &passed);
    } else {
      ret = ds4->nocomp_op(pos, &nocomp_tuple, &passed);
    }
    if (!passed) { return 0; }

    if (column_id == dsv.at(table_id).size() && table_id == num_tables-1) { return 0; }

    task *t2 = NULL;
    t2 = new task();
    if (column_id != dsv.at(table_id).size()) {
      t2->table_id = table_id;
      t2->column_id = column_id+1;
      t2->pos = pos;
      t2->iofuncp = nlj_em_cio2;
      t2->opfuncp = nlj_em_cop1;
    } else {
      t2->table_id = table_id+1;
      t2->column_id = 0;
      t2->pos = pos;
      //t2->val = new char[8];
      if (ds4->get_comp_type() == cstore::RLE) {
        memcpy(&(t2->val), rle_tuple.val, 8);
      } else {
        memcpy(&(t2->val), nocomp_tuple.val, 8);
      }
      t2->col_type = ds4->get_col_type();
      t2->iofuncp = nlj_em_cio2;
      t2->opfuncp = nlj_em_cop2; // not used
    }

    //pthread_mutex_lock(&counter_mutex);
    //pthread_mutex_unlock(&counter_mutex);
    pthread_mutex_lock(&mutex);
    produced[t2->table_id][t2->column_id]++;
    q.push_back(t2);
    pthread_mutex_unlock(&mutex);
      
  }
}

int nlj_lm_cio2(int tid, task *t)
{
  uint32_t table_id = t->table_id;
  uint32_t column_id = t->column_id;
  //std::cout << "table: " << table_id << ", io for column: " << column_id << ", pos: " << t->pos << std::endl;

  if (column_id == 0) {
    // nlj inner table index lookup by a specified value
    Db *idx = idxes.at(table_id).at(tid);
    Dbt idx_k;
    Dbt idx_v;
    memset(&idx_k, 0, sizeof(Dbt)); 
    memset(&idx_v, 0, sizeof(Dbt)); 

    Dbc *cursorp;
    //cstore::rle_triple_tuple_t t;
    try {
      idx->cursor(NULL, &cursorp, 0);
      off_t pos;
      idx_k.set_data(&(t->val));
      idx_k.set_size(cstore::get_col_size(t->col_type));
      idx_v.set_data(&pos);
      idx_v.set_ulen(sizeof(off_t));
      idx_v.set_flags(DB_DBT_USERMEM);
      int ret = cursorp->get(&idx_k, &idx_v, DB_SET);
      do {
        // TODO: TO FIX
        if (ret != 0 || cstore::generic_compare(idx_k.get_data(), &(t->val), t->col_type) != 0) {
          if (ret == DB_NOTFOUND) {
            std::cout << "not found" << std::endl;
          }
          break;
        }
        task *t2 = new task();
        t2->table_id = table_id;
        t2->column_id = 1;
        t2->pos = pos;
        t2->iofuncp = nlj_lm_cio2;
        t2->opfuncp = nlj_em_cop2;
        // TODO: should add pos for LM (poses are managed as an array for tables)
        std::vector<off_t> poses;
        for (int i = 0; i < t->lm_poses.size(); ++i) {
          poses.push_back(t->lm_poses.at(i));
        }
        poses.push_back(pos);
        t2->lm_poses = poses;

        //pthread_mutex_lock(&counter_mutex);
        //pthread_mutex_unlock(&counter_mutex);

        pthread_mutex_lock(&mutex);
        produced[t2->table_id][t2->column_id]++;
        q.push_back(t2);
        pthread_mutex_unlock(&mutex);
        ret = cursorp->get(&idx_k, &idx_v, DB_NEXT);
      } while (true);

    } catch(DbException &e) {
      idx->err(e.get_errno(), "Error!");
    } catch(std::exception &e) {
      idx->errx("Error! %s", e.what());
    }
    cursorp->close();

  } else {
    // tuple reconstruction
    // 0 (w/ predicate), 1 (next JK), 2- (others)
    off_t pos = t->pos;
    cstore::nocomp_tuple_t nocomp_tuple;
    cstore::rle_triple_tuple_t rle_tuple;
    bool passed;
    bool ret;

    cstore::DS4 *ds4 = dsv.at(table_id).at(column_id-1).at(tid);
    if (ds4->get_comp_type() == cstore::RLE) {
      ret = ds4->rle_op(pos, &rle_tuple, &passed);
    } else {
      ret = ds4->nocomp_op(pos, &nocomp_tuple, &passed);
    }
    if (!passed) { return 0; }
    if (column_id == 2 && table_id == num_tables-1) {
      // late materialization
      for (int i = 0; i < t->lm_poses.size(); ++i) {
        // column_id:2-N for late lookups
        // column_id:3-N for late lookups
        task *t2 = NULL;
        t2 = new task();
        t2->table_id = i;
        if (i == 0) {
          t2->column_id = column_id;
          t2->iofuncp = nlj_lm_cio1;
        } else {
          t2->column_id = column_id+1;
          t2->iofuncp = nlj_lm_cio2;
        }
        t2->pos = t->lm_poses.at(i);
        t2->opfuncp = nlj_em_cop1;
        t2->lm_poses = t->lm_poses;
        if (t2->column_id > dsv.at(i).size()) {
          continue;
        }
        //pthread_mutex_lock(&counter_mutex);
        //pthread_mutex_unlock(&counter_mutex);
        pthread_mutex_lock(&mutex);
        produced[t2->table_id][t2->column_id]++;
        q.push_back(t2);
        pthread_mutex_unlock(&mutex);
      }
      return 0;
    } else if (column_id == 2) {
      // next join
      task *t2 = NULL;
      t2 = new task();
      t2->table_id = table_id+1;
      t2->column_id = 0;
      t2->pos = pos;
      //t2->val = new char[8];
      if (ds4->get_comp_type() == cstore::RLE) {
        memcpy(&(t2->val), rle_tuple.val, 8);
      } else {
        memcpy(&(t2->val), nocomp_tuple.val, 8);
      }
      t2->col_type = ds4->get_col_type();
      t2->iofuncp = nlj_lm_cio2;
      t2->opfuncp = nlj_em_cop2; // not used
      t2->lm_poses = t->lm_poses;
      //pthread_mutex_lock(&counter_mutex);
      //pthread_mutex_unlock(&counter_mutex);
      pthread_mutex_lock(&mutex);
      produced[t2->table_id][t2->column_id]++;
      q.push_back(t2);
      pthread_mutex_unlock(&mutex);
      return 0;
    }
    if (column_id == dsv.at(table_id).size()) { return 0; } 

    task *t2 = NULL;
    t2 = new task();
    t2->table_id = table_id;
    t2->column_id = column_id+1;
    t2->pos = pos;
    t2->iofuncp = nlj_lm_cio2;
    t2->opfuncp = nlj_em_cop2;
    t2->lm_poses = t->lm_poses;

    //pthread_mutex_lock(&counter_mutex);
    //pthread_mutex_unlock(&counter_mutex);
    pthread_mutex_lock(&mutex);
    produced[t2->table_id][t2->column_id]++;
    q.push_back(t2);
    pthread_mutex_unlock(&mutex);
      
  }
}

int opempty(int tid, task *t)
{
}

int iolookup(int tid, task *t)
{
  uint32_t table_id = t->table_id;
  uint32_t part_id = t->part_id;
  uint32_t column_id = t->column_id;
  off_t pos = t->pos;
  //std::cout << "LM-lookup table: " << table_id << ", io for column: " << column_id << ", pos: " << pos << std::endl;
  cstore::nocomp_tuple_t nocomp_tuple;
  cstore::rle_triple_tuple_t rle_tuple;
  bool passed;
  bool ret;

  cstore::DS4 *ds4 = dsv.at(table_id).at(column_id-1).at(tid);
  if (ds4->get_comp_type() == cstore::RLE) {
    ret = ds4->rle_op(pos, &rle_tuple, &passed);
  } else {
    ret = ds4->nocomp_op(pos, &nocomp_tuple, &passed);
  }
  if (!passed) { return 0; }
  if (t->column_id + 1 >= num_columns[0]) { return 0; }

  task *t2 = new task();
  t2->table_id = t->table_id;
  t2->part_id = t->part_id;
  t2->column_id = t->column_id + 1;
  t2->pos = t->pos;
  t2->iofuncp = iolookup;
  t2->opfuncp = opempty;
  pthread_mutex_lock(&mutex);
  produced[t2->table_id][t2->column_id]++;
  q.push_back(t2);
  pthread_mutex_unlock(&mutex);
  return 0;
}

void scan_em_start1(int table_id)
{
  pthread_t readers[parallelism];
  sarg args[parallelism];
  for (int i = 0; i < parallelism; ++i) {
    args[i].part_id = i;
    args[i].table_id = table_id;
    if (pthread_create(&readers[i], NULL, scan_em_start1_part, (void *) &args[i]) != 0) {
      perror("pthread_create");
      exit(1);
    }
  }

  void *ret = NULL;
  for (int i = 0; i < parallelism; ++i) {
    if (pthread_join(readers[i], &ret)) {
      perror("pthread_join");
    }
  }
  for (uint64_t i = 0; i < num_columns[0]; i++) {
    std::cout << "produced[0][" << i << "] = " << produced[0][i] << ", done[0][" << i << "] = " << done[0][i] << std::endl;
  }
  std::cout << "scanned 0" << std::endl;
}

void *scan_em_start1_part(void *p)
{
  sarg *arg = (sarg *) p;
  int part_id = arg->part_id;
  int table_id = arg->table_id;

  cstore::DS2 *ds2 = scanss.at(0).at(part_id);
  off_t pos = 0;
  char *val1;
  bool ret;
  bool passed;
  while (true) {
    if (ds2->get_comp_type() == cstore::RLE) {
      ret = ds2->rle_op(&pos, (void **) &val1);
    } else {
      ret = ds2->nocomp_op(&pos, (void **) &val1);
    }
    done[0][0]++;
    if (!ret) { break; }

    // other columns to materialize
    if (strncmp(ooo, "on", 2) == 0) {
      // out-of-order point lookups
      /*
      for (int i = 1; i < num_columns[0]; ++i) {
        task *t = new task();
        t->table_id = 0;
        t->part_id = part_id;
        t->column_id = i;
        t->pos = pos;
        t->iofuncp = iolookup;
        t->opfuncp = opempty;
        pthread_mutex_lock(&mutex);
        q.push_back(t);
        pthread_mutex_unlock(&mutex);
      }
      */
      task *t = new task();
      t->table_id = 0;
      t->part_id = part_id;
      t->column_id = 1;
      t->pos = pos;
      t->iofuncp = iolookup;
      t->opfuncp = opempty;
      pthread_mutex_lock(&mutex);
      q.push_back(t);
      pthread_mutex_unlock(&mutex);
    } else {
      // usual in-order scans
      for (int i = 1; i < num_columns[0]; ++i) {
        cstore::rle_triple_tuple_t rle_tuple;
        cstore::nocomp_tuple_t *nocomp_tuple;
        cstore::DS4 *ds4 = dsv.at(0).at(i-1).at(part_id);
        if (ds4->get_comp_type() == cstore::RLE) {
          ret = ds4->rle_op(pos, &rle_tuple, &passed);
        } else {
          ret = ds4->nocomp_op_new(pos, (void **) &nocomp_tuple, &passed);
        }
        done[0][i]++;
        if (!passed) { break; }
      }
    }
  }
  if (strncmp(ooo, "on", 2) == 0) {
    // wait for lookups are finished
    while (true) {
      pthread_mutex_lock(&mutex);
      // NOTICE: outstanding ones are ignored for now
      if (q.empty()) {
        pthread_mutex_unlock(&mutex);
        break;
      }
      pthread_mutex_unlock(&mutex);
      usleep(100000);
    }
  }
}

void scan_lm_start1(int table_id)
{
  pthread_t readers[parallelism];
  sarg args[parallelism];
  for (int i = 0; i < parallelism; ++i) {
    args[i].part_id = i;
    args[i].table_id = table_id;
    if (pthread_create(&readers[i], NULL, scan_lm_start1_part, (void *) &args[i]) != 0) {
      perror("pthread_create");
      exit(1);
    }
  }

  void *ret = NULL;
  for (int i = 0; i < parallelism; ++i) {
    if (pthread_join(readers[i], &ret)) {
      perror("pthread_join");
    }
  }
  std::cout << "scanned 0" << std::endl;
}

void *scan_lm_start1_part(void *p)
{
  sarg *arg = (sarg *) p;
  int part_id = arg->part_id;
  int table_id = arg->table_id;

  cstore::DS2 *ds2 = scanss.at(0).at(part_id);
  std::vector<off_t> poses;
  off_t pos = 0;
  char *val1;
  bool ret;
  while (true) {
    if (ds2->get_comp_type() == cstore::RLE) {
      ret = ds2->rle_op(&pos, (void **) &val1);
    } else {
      ret = ds2->nocomp_op(&pos, (void **) &val1);
    }
    if (!ret) { break; }
    poses.push_back(pos);
  }

  bool passed;
  for (int i = 0; i < poses.size(); ++i) {
    off_t pos = poses.at(i);
    
    // other columns to materialize
    if (strncmp(ooo, "on", 2) == 0) {
      // out-of-order point lookups
      /*
      for (int i = 1; i < num_columns[0]; ++i) {
        task *t = new task();
        t->table_id = 0;
        t->part_id = part_id;
        t->column_id = i;
        t->pos = pos;
        t->iofuncp = iolookup;
        t->opfuncp = opempty;
        pthread_mutex_lock(&mutex);
        q.push_back(t);
        pthread_mutex_unlock(&mutex);
      }
      */
      task *t = new task();
      t->table_id = 0;
      t->part_id = part_id;
      t->column_id = 1;
      t->pos = pos;
      t->iofuncp = iolookup;
      t->opfuncp = opempty;
      pthread_mutex_lock(&mutex);
      q.push_back(t);
      pthread_mutex_unlock(&mutex);
    } else {
      // usual in-order scans
      for (int i = 1; i < num_columns[0]; ++i) {
        cstore::rle_triple_tuple_t rle_tuple;
        cstore::nocomp_tuple_t *nocomp_tuple;
        cstore::DS4 *ds4 = dsv.at(0).at(i-1).at(part_id);
        if (ds4->get_comp_type() == cstore::RLE) {
          ret = ds4->rle_op(pos, &rle_tuple, &passed);
        } else {
          ret = ds4->nocomp_op_new(pos, (void **) &nocomp_tuple, &passed);
        }
        if (!passed) { break; }
      }
    }
  }
  if (strncmp(ooo, "on", 2) == 0) {
    // wait for lookups are finished
    while (true) {
      pthread_mutex_lock(&mutex);
      // NOTICE: outstanding ones are ignored for now
      if (q.empty()) {
        pthread_mutex_unlock(&mutex);
        break;
      }
      pthread_mutex_unlock(&mutex);
      usleep(100000);
    }
  }
}

void nlj_em_start1(int table_id)
{
  // TODO: TO FIX
  // assumes that index predicate is applied as LE

  Db *idx = idxes.at(0).at(0);

  Dbt idx_k;
  Dbt idx_v;
  memset(&idx_k, 0, sizeof(Dbt)); 
  memset(&idx_v, 0, sizeof(Dbt)); 
  idx_k.set_size(cstore::get_col_size(pred_col_types.at(0)));

  Dbc *cursorp;
  //cstore::rle_triple_tuple_t t;
  try {
    idx->cursor(NULL, &cursorp, 0);
    off_t pos;
    //int64_t a = 0;
    //idx_k.set_data(&a);
    void *pred = preds.at(0);
    idx_k.set_data(pred);
    idx_v.set_data(&pos);
    idx_v.set_ulen(sizeof(off_t));
    idx_v.set_flags(DB_DBT_USERMEM);
    int ret;
    ret = cursorp->get(&idx_k, &idx_v, DB_SET);
    cstore::col_t pred_col_type = pred_col_types.at(0);
    do {
      // TODO: TO FIX
      //if (ret != 0 || cstore::generic_compare(idx_k.get_data(), pred, pred_col_type) > 0) {
      if (ret != 0 || cstore::generic_compare(idx_k.get_data(), pred, pred_col_type) != 0) {
      //if (ret != 0) {
        if (ret == DB_NOTFOUND) {
          std::cout << "not found" << std::endl;
        }
        break;
      }
      task *t = new task();
      t->table_id = 0;
      t->column_id = 1;
      t->pos = pos;
      if (is_lm) {
        std::vector<off_t> poses;
        t->lm_poses = poses;
        t->iofuncp = nlj_lm_cio1;
        t->opfuncp = nlj_em_cop1;
      } else {
        t->iofuncp = nlj_em_cio1;
        t->opfuncp = nlj_em_cop1;
      }
      //pthread_mutex_lock(&counter_mutex);
      //pthread_mutex_unlock(&counter_mutex);

      pthread_mutex_lock(&mutex);
      produced[0][t->column_id]++;
      //q.push_back(t);
      q1.push_back(t);
      pthread_mutex_unlock(&mutex);
      ret = cursorp->get(&idx_k, &idx_v, DB_NEXT);
    } while (true);

  } catch(DbException &e) {
    idx->err(e.get_errno(), "Error!");
  } catch(std::exception &e) {
    idx->errx("Error! %s", e.what());
  }
  cursorp->close();
  idx->close(0);
}

// table2 start-up function
void nlj_em_start2(int table_id)
{
}

std::vector<std::string> hj_prepare_outer(char *table_def, std::vector<char *> table_preds, std::vector<char *> table_comps,
                                           std::vector< std::vector<cstore::DS4 *> > &dsv,
                                           std::vector<void *> &preds,
                                           std::vector< std::vector<Db *> > &idxes,
                                           std::vector<cstore::col_t> &pred_col_types)
{
  std::ifstream ifs(table_def);
  if (!ifs) {
    std::cerr << "can't open " << table_def << std::endl;
    exit(1);
  }
  std::vector<std::string> columns;
  std::string line;
  while (getline(ifs, line)) {
    if (line.find_first_of("#") == 0) {
      continue;
    }
    columns.push_back(line);
  }

  std::vector<cstore::DS2 *> scans;
  if (parallelism == 1) {
    cstore::Column *c = new cstore::Column();
    if (!c->open(columns.at(0), COL_RDONLY)) {
      perror(columns.at(0).c_str());
      exit(1);
    }
    void *pred = NULL;
    if (strcmp(table_preds.at(0), "NULL") != 0) {
      pred = cstore::get_pred(table_preds.at(0), c->get_col_type());
    }
    preds.push_back(pred);
    pred_col_types.push_back(c->get_col_type());
    // NOTICE: default 64K buffer is good enough ?
    scans.push_back(new cstore::DS2(c, pred, cstore::get_compare_type(table_comps.at(0))));
  } else {
    for (int i = 0; i < parallelism; ++i) {
      cstore::Column *c = new cstore::Column();
      std::stringstream cname;
      cname << columns.at(0) << "." << i;
      if (!c->open(cname.str(), COL_RDONLY)) {
        perror(columns.at(0).c_str());
        exit(1);
      }
      void *pred = NULL;
      if (strcmp(table_preds.at(0), "NULL") != 0) {
        pred = cstore::get_pred(table_preds.at(0), c->get_col_type());
      }
      preds.push_back(pred);
      pred_col_types.push_back(c->get_col_type());
      // NOTICE: default 64K buffer is good enough ?
      scans.push_back(new cstore::DS2(c, pred, cstore::get_compare_type(table_comps.at(0))));
    }
  }
  scanss.push_back(scans);

  for (int i = 1; i < columns.size(); ++i) {
    std::vector<cstore::DS4 *> ds;
    if (parallelism == 1) {
      for (int j = 0; j < num_threads; ++j) {
        cstore::Column *c = new cstore::Column();
        if (!c->open(columns.at(i), COL_RDONLY)) {
          perror(columns.at(i).c_str());
          exit(1);
        }

        void *pred = NULL;
        if (table_preds.size() > i) {
          pred = cstore::get_pred(table_preds.at(i), c->get_col_type());
          preds.push_back(pred);
          pred_col_types.push_back(c->get_col_type());
          ds.push_back(new cstore::DS4(c, pred, cstore::get_compare_type(table_comps.at(i))));
        } else {
          //ds.push_back(new cstore::DS4(c, NULL, cstore::EQ, 65536));
          ds.push_back(new cstore::DS4(c, NULL, cstore::EQ));
        }
      }
    } else {
      int para = (num_threads > parallelism) ? num_threads : parallelism;
      for (int j = 0; j < para; ++j) {
        int foff = j % parallelism;
        std::stringstream cname;
        cname << columns.at(i) << "." << foff;
        cstore::Column *c = new cstore::Column();
        if (!c->open(cname.str(), COL_RDONLY)) {
          perror(cname.str().c_str());
          exit(1);
        }
        void *pred = NULL;
        if (table_preds.size() > i) {
          pred = cstore::get_pred(table_preds.at(i), c->get_col_type());
          preds.push_back(pred);
          pred_col_types.push_back(c->get_col_type());
          ds.push_back(new cstore::DS4(c, pred, cstore::get_compare_type(table_comps.at(i))));
        } else {
          //ds.push_back(new cstore::DS4(c, NULL, cstore::EQ, 65536));
          ds.push_back(new cstore::DS4(c, NULL, cstore::EQ));
        }
      } 
    }
    dsv.push_back(ds);
  }

  return columns;
}

std::vector<std::string> nlj_prepare_outer(char *table_def, std::vector<char *> table_preds, std::vector<char *> table_comps, 
                                           std::vector< std::vector<cstore::DS4 *> > &dsv,
                                           std::vector<void *> &preds,
                                           std::vector< std::vector<Db *> > &idxes,
                                           std::vector<cstore::col_t> &pred_col_types)
{
  std::ifstream ifs(table_def);
  if (!ifs) {
    std::cerr << "can't open " << table_def << std::endl;
    exit(1);
  }
  std::vector<std::string> columns;
  std::string line;
  while (getline(ifs, line)) {
    if (line.find_first_of("#") == 0) {
      continue;
    }
    columns.push_back(line);
  }

  u_int32_t oFlags = DB_RDONLY;
  std::string idx_name = columns.at(0) + ".idx";
  Db *idx = new Db(NULL, (u_int32_t) 0); 
  //idx->set_bt_compare(cstore::compare_int64);
  if (idx->open(NULL, idx_name.c_str(), NULL, DB_BTREE, oFlags, 0) != 0) {
    std::cerr << "opening " << idx_name << " failed." << std::endl;
    exit(1);
  }
  std::vector<Db *> idxv;
  idxv.push_back(idx);
  idxes.push_back(idxv);

  cstore::Column *c = new cstore::Column();
  if (!c->open(columns.at(0), COL_RDONLY)) {
    perror(columns.at(0).c_str());
    exit(1);
  }
  void *pred = NULL;
  if (strcmp(table_preds.at(0), "NULL") != 0) {
    pred = cstore::get_pred(table_preds.at(0), c->get_col_type());
  }
  preds.push_back(pred);
  pred_col_types.push_back(c->get_col_type());

  for (int i = 1; i < columns.size(); ++i) {
    std::vector<cstore::DS4 *> ds;

    if (table_preds.size() > i) {
      pred = cstore::get_pred(table_preds.at(i), c->get_col_type());
      preds.push_back(pred);
      pred_col_types.push_back(c->get_col_type());

      for (int j = 0; j < num_threads; ++j) {
        cstore::Column *c = new cstore::Column();
        if (!c->open(columns.at(i), COL_RDONLY)) {
          perror(columns.at(i).c_str());
          exit(1);
        }
        //ds.push_back(new cstore::DS4(c, NULL, cstore::EQ));
        ds.push_back(new cstore::DS4(c, pred, cstore::EQ));
        dsv.push_back(ds);
      }
    } else {
      for (int j = 0; j < num_threads; ++j) {
        cstore::Column *c = new cstore::Column();
        if (!c->open(columns.at(i), COL_RDONLY)) {
          perror(columns.at(i).c_str());
          exit(1);
        }
        ds.push_back(new cstore::DS4(c, NULL, cstore::EQ));
        dsv.push_back(ds);
      }
    }
  }

  return columns;
}

