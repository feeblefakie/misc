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

void index_em_start1(int table_id);
void scan_em_start1(int table_id);
void *scan_em_start1_part(void *p);
void scan_lm_start1(int table_id);
void *scan_lm_start1_part(void *p);

int opempty(int tid, task *t);
int iolookup(int tid, task *t);
void eval_pred(off_t pos, off_t *start, off_t *end);

std::vector<std::string> index_prepare_outer(char *table_def, std::vector<char *> table_preds, std::vector<char *> table_comps, 
                                           std::vector< std::vector<cstore::DS4 *> > &dsv,
                                           std::vector<void *> &preds,
                                           std::vector< std::vector<Db *> > &idxes,
                                           std::vector<cstore::col_t> &pred_col_types);
std::vector<std::string> scan_prepare_outer(char *table_def, std::vector<char *> table_preds, std::vector<char *> table_comps,
                                           std::vector< std::vector<cstore::DS4 *> > &dsv,
                                           std::vector<void *> &preds,
                                           std::vector< std::vector<Db *> > &idxes,
                                           std::vector<cstore::col_t> &pred_col_types);

table_func nlj_em[] = {
  {index_em_start1, index_prepare_outer}
};

table_func scan_em[] = {
  {scan_em_start1, scan_prepare_outer}
};

table_func scan_lm[] = {
  {scan_lm_start1, scan_prepare_outer}
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
std::unordered_map<int64_t, bool> h1;
std::unordered_map<int64_t, bool> h2;
std::unordered_map<int64_t, bool> hs[2] = {h1, h2};

char *ooo;
int parallelism = 1; // for HJ

int main(int argc, char *argv[]) {

  if (argc < 8) {
    std::cerr << "Usage: " << argv[0] << " join_method(S|I) materialization(EM|LM) out-of-order(on|off) num_threads table_def num_preds pred1 comp1 [pred2 comp2]" << std::endl;
    exit(1);
  }
  char *join = argv[1];
  char *mat = argv[2];
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
  if (strncmp(join, "I", 1) == 0) {
    if (strncmp(mat, "EM", 2) == 0) {
      // Index-EM
      for (int i = 0; i < num_tables; ++i) {
        f[i] = nlj_em[i];
      }
    } else {
      // Index-LM (exists ?)
      for (int i = 0; i < num_tables; ++i) {
        f[i] = nlj_em[i];
      }
    }
  } else {
    if (strncmp(mat, "EM", 2) == 0) {
      // Scan-EM
      f[0] = scan_em[0];
    } else {
      // Scan-LM
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
        std::cout << "produced[0][" << i << "] = " << produced[0][i] << ", done[0][" << i << "] = " << done[0][i] << std::endl;
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

int opempty(int tid, task *t)
{
}

int iolookup(int tid, task *t)
{
  // TODO
  // check if the task needs some query-dependent exceptional processing
  // Ex. if (column_id == 3) { // scan the column until seeing speed > 30; }
  
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
  int nresults = 0;
  while (true) {
    produced[0][0]++;
    if (ds2->get_comp_type() == cstore::RLE) {
      ret = ds2->rle_op(&pos, (void **) &val1);
    } else {
      ret = ds2->nocomp_op(&pos, (void **) &val1);
    }
    done[0][0]++;
    if (!ret) { break; }
    //std::cout << "val: " << *(uint32_t *) val1 << std::endl;
    //std::cout << "pos: " << pos << std::endl;

    off_t start = 0;
    off_t end = 0;
    // TODO: 
    eval_pred(pos, &start, &end);

    if (start == end) {
      continue;
    }
    std::cout << "range start: " << start << " - end: " << end << ", size: " << end - start + 1 << std::endl;
    nresults += end - start + 1;

    // pos should be set to end so that duplicate lookups are avoided
    ds2->set_offset(end + 1);
    
    for (int j = start; j < end; j++) {
      // other columns to materialize
      if (strncmp(ooo, "on", 2) == 0) {
        produced[0][4]++;
        // out-of-order point lookups
        task *t = new task();
        t->table_id = 0;
        t->part_id = part_id;
        t->column_id = 4;
        t->pos = j;
        t->iofuncp = iolookup;
        t->opfuncp = opempty;
        pthread_mutex_lock(&mutex);
        q.push_back(t);
        pthread_mutex_unlock(&mutex);
      } else {
        // usual in-order scans
        for (int i = 4; i < num_columns[0]; ++i) {
          produced[0][i]++;
          cstore::rle_triple_tuple_t rle_tuple;
          cstore::nocomp_tuple_t *nocomp_tuple;
          cstore::DS4 *ds4 = dsv.at(0).at(i-1).at(part_id);
          if (ds4->get_comp_type() == cstore::RLE) {
            ret = ds4->rle_op(j, &rle_tuple, &passed);
          } else {
            ret = ds4->nocomp_op_new(j, (void **) &nocomp_tuple, &passed);
          }
          done[0][i]++;
          if (!passed) { break; }
        }
      }
    }
  }
  std::cout << "nresults: " << nresults << std::endl;

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

void index_em_start1(int table_id)
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
      t->iofuncp = iolookup;
      t->opfuncp = opempty;
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

std::vector<std::string> scan_prepare_outer(char *table_def, std::vector<char *> table_preds, std::vector<char *> table_comps,
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

std::vector<std::string> index_prepare_outer(char *table_def, std::vector<char *> table_preds, std::vector<char *> table_comps, 
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

// TODO 
// speed == 0 and 
// brake > 0 and
// scan backward until not brake != 0 (and get the pos_start) and 
// scan forward until speed > 30  (and get the pos_end) and
// retrieve attributes between pos_start and pos_end in OoO fashion.

void eval_pred(off_t pos, off_t *start, off_t *end)
{
  bool ret;
  bool passed ;

  // brake > 0
  cstore::rle_triple_tuple_t rle_tuple;
  cstore::nocomp_tuple_t *nocomp_tuple;
  cstore::DS4 *ds4 = dsv.at(0).at(0).at(0);
  produced[0][1]++;
  if (ds4->get_comp_type() == cstore::RLE) {
    ret = ds4->rle_op(pos, &rle_tuple, &passed);
  } else {
    ret = ds4->nocomp_op_new(pos, (void **) &nocomp_tuple, &passed);
  }
  done[0][1]++;
  if (!passed) { return; }

  // scan backward until not brake != 0 (and get the pos_start) 
  *start = pos;
  ds4 = dsv.at(0).at(1).at(0);
  do {
    produced[0][2]++;
    (*start)--;
    if (ds4->get_comp_type() == cstore::RLE) {
      ret = ds4->rle_op(*start, &rle_tuple, &passed);
    } else {
      ret = ds4->nocomp_op_new(*start, (void **) &nocomp_tuple, &passed);
    }
    done[0][2]++;
    if (!ret) { break; }
  } while (passed);
  (*start)++;

  // scan forward until speed > 30  (and get the pos_end)
  *end = pos;
  ds4 = dsv.at(0).at(2).at(0);
  do {
    produced[0][3]++;
    (*end)++;
    if (ds4->get_comp_type() == cstore::RLE) {
      ret = ds4->rle_op(*end, &rle_tuple, &passed);
    } else {
      ret = ds4->nocomp_op_new(*end, (void **) &nocomp_tuple, &passed);
    }
    //std::cout << "speed: " << *(uint32_t *) nocomp_tuple->val << std::endl;
    done[0][3]++;
    if (!ret) { break; }
  } while (passed);
}
