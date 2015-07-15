#include <iostream>
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
#include "ops.h"
#include "query_util.h"

typedef struct {
  off_t pos;
  off_t off;
} element_t;

typedef struct {
  int id;
  std::deque<element_t *> *q;
} thread_arg_t;

void *pos_lookuper(void *p);
static double gettimeofday_sec();

int num_threads;
int global_cnt = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<std::vector<cstore::DS4 *> > dsv;
int num_sensors;
off_t *produced;
off_t *consumed;
bool *done;
off_t done_offset;
bool index_traversed = false;
bool using_index = false;
cstore::compare_t col2_compare_type;

int main(int argc, char *argv[]) {

  if (argc != 13) {
    std::cerr << "Usage: " << argv[0] << " col1_name col2_name col1_pred col2_pred col1_compare col2_compare ";
    std::cerr << "#_of_1ms_samples #_of_10ms_samples #_of_100ms_samples #_of_1000ms_samples num_thread using_index(true|false)" << std::endl;
    exit(1);
  }
  char *col1_name = argv[1];
  char *col2_name = argv[2];
  char *col1_pred = argv[3];
  char *col2_pred = argv[4];
  cstore::compare_t col1_compare_type = cstore::get_compare_type(argv[5]);
  col2_compare_type = cstore::get_compare_type(argv[6]);
  int num_mss[4];
  num_mss[0] = atoi(argv[7]); // 1ms
  num_mss[1] = atoi(argv[8]); // 10ms
  num_mss[2] = atoi(argv[9]); // 100ms
  num_mss[3] = atoi(argv[10]); // 1000ms
  num_sensors = num_mss[0] + num_mss[1] + num_mss[2] + num_mss[3];
  num_threads = atoi(argv[11]);
  std::cout << "num_threads: " << num_threads << std::endl;
  if (strcmp(argv[12], "true") == 0) {
    using_index = true;
  } else {
    using_index = false;
  }

  produced = new off_t[num_sensors+2];
  consumed = new off_t[num_sensors+2];
  done = new bool[num_sensors+2];
  done_offset = 0;

  double t1 = gettimeofday_sec();

  std::vector<cstore::DS4 *> dss1;
  cstore::Column *c1 = new cstore::Column();
  if (!c1->open(col1_name, COL_RDONLY)) {
    std::cerr << "can't open " << col1_name << std::endl;
    perror(col1_name);
    exit(1);
  }
  void *pred1 = NULL;
  if (strcmp(col1_pred, "NULL") != 0) {
    pred1 = cstore::get_pred(col1_pred, c1->get_col_type());
  }
  cstore::DS2 *ds2 = new cstore::DS2(c1, pred1, col1_compare_type);

  if (using_index) {
    for (int j = 0; j < num_threads; ++j) {
      cstore::Column *c1 = new cstore::Column();
      if (!c1->open(col1_name, COL_RDONLY)) {
        std::cerr << "can't open " << col1_name << std::endl;
        perror(col1_name);
        exit(1);
      }
      // pred1 is applied through index.
      cstore::DS4 *ds4 = new cstore::DS4(c1, NULL, col1_compare_type);
      dss1.push_back(ds4);
    }
  }
  dsv.push_back(dss1);

  std::vector<cstore::DS4 *> dss2;
  for (int j = 0; j < num_threads; ++j) {
    cstore::Column *c2 = new cstore::Column();
    if (!c2->open(col2_name, COL_RDONLY)) {
      std::cerr << "can't open " << col2_name << std::endl;
      perror(col2_name);
      exit(1);
    }
    void *pred2 = NULL;
    if (strcmp(col2_pred, "NULL") != 0) {
      pred2 = cstore::get_pred(col2_pred, c2->get_col_type());
    }
    cstore::DS4 *ds4 = new cstore::DS4(c2, pred2, col2_compare_type);
    dss2.push_back(ds4);
  }
  dsv.push_back(dss2);

  for (int k = 0; k < 4; ++k) {
    for (int i = 0; i < num_mss[k]; ++i) {
      std::vector<cstore::DS4 *> dss3;
      for (int j = 0; j < num_threads; ++j) {
        cstore::Column *c3= new cstore::Column();
        char buf[128];
        sprintf(buf, "./samples/sample%d.%d.col", (int) pow(10, k), i+3);
        if (!c3->open(buf, COL_RDONLY)) {
          std::cerr << "can't open " << buf << std::endl;
          perror(buf);
          exit(1);
        }
        cstore::DS4 *ds4s1 = new cstore::DS4(c3, NULL, cstore::EQ);
        dss3.push_back(ds4s1);
      }
      dsv.push_back(dss3);
    }
  }

  double t2 = gettimeofday_sec();
  std::cout << "start-up time: " << t2 - t1 << std::endl;

  // create lookup threads
  std::deque<element_t *> queues;
  pthread_t readers[num_threads];
  thread_arg_t arg[num_threads];
  for (int i = 0; i < num_threads; ++i) {
    arg[i].id = i;
    arg[i].q = &queues;
    if (pthread_create(&readers[i], NULL, pos_lookuper, (void *) &arg[i]) != 0) {
      perror("pthread_create");
      exit(1);
    }
  }

  bool ret;
  bool passed;
  char *val1, *val2, *val3;
  off_t pos = 0;
  off_t start, length;
  off_t n_pos_lookups = 0;
  off_t n_tuples = 0;

  if (!using_index) {
    while (true) {
      // paper version
      if (ds2->get_comp_type() == cstore::RLE) {
        ret = ds2->rle_op(&pos, (void **) &val1);
      } else {
        ret = ds2->nocomp_op(&pos, (void **) &val1);
      }
      if (!ret) {
        pthread_mutex_lock(&mutex);
        done[0] = true;
        done_offset = 1;
        pthread_mutex_unlock(&mutex);
        std::cout << "first done" << std::endl;
        std::cout.flush();
        break;
      } else {
        pthread_mutex_lock(&mutex);
        produced[0]++;
        consumed[0]++;
        produced[1]++;
        pthread_mutex_unlock(&mutex);
      }
      element_t *elem = new element_t();
      elem->pos = pos;
      elem->off = 1;
      pthread_mutex_lock(&mutex);
      queues.push_back(elem);
      pthread_mutex_unlock(&mutex);
    }
  } else {

    // TODO: TO FIX
    // assumes that index predicate is applied as LE

    // via index
    u_int32_t oFlags = DB_RDONLY;
    std::string idx_name = std::string(col1_name) + ".idx";
    Db *idx = new Db(NULL, (u_int32_t) 0); 
    idx->set_bt_compare(cstore::compare_int64);
    if (idx->open(NULL, idx_name.c_str(), NULL, DB_BTREE, oFlags, 0) != 0) {
      std::cerr << "opening " << idx_name << " failed." << std::endl;
      exit(1);
    }

    Dbt idx_k;
    Dbt idx_v;
    memset(&idx_k, 0, sizeof(Dbt)); 
    memset(&idx_v, 0, sizeof(Dbt)); 
    idx_k.set_size(sizeof(int64_t));
    idx_v.set_size(sizeof(off_t));

    Dbc *cursorp;
    //cstore::rle_triple_tuple_t t;
    try {
      idx->cursor(NULL, &cursorp, 0);
      off_t pos;
      int64_t a = 0;
      idx_k.set_data(&a);
      //idx_k.set_data(pred1);
      idx_v.set_data(&pos);
      idx_v.set_ulen(sizeof(off_t));
      idx_v.set_flags(DB_DBT_USERMEM);
      int ret;
      ret = cursorp->get(&idx_k, &idx_v, DB_SET);
      int cnt = 0;
      do {
        // TODO: TO FIX
        if (ret != 0 || *(int64_t *) idx_k.get_data() > *(int64_t *) pred1) {
          ///*
          std::cout << "ret = " << ret << std::endl;
          if (ret == DB_NOTFOUND) {
            std::cout << "not found" << std::endl;
          }
          //*/
          break;
        }
        produced[0]++;
        element_t *elem = new element_t();
        elem->pos = pos;
        elem->off = 0;
        pthread_mutex_lock(&mutex);
        queues.push_back(elem);
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
  index_traversed = true;

  void *r = NULL;
  for (int i = 0; i < num_threads; ++i) {
    if (pthread_join(readers[i], &r)) {
      perror("pthread_join");
    }
  }

  std::cerr << "n_tuples: " << n_tuples << std::endl;

  delete(c1);
  //delete(c2);

  return 0;
}

static double 
gettimeofday_sec()
{
  struct timeval tv; 
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void *pos_lookuper(void *p)
{
  thread_arg_t *arg = (thread_arg_t *) p;
  std::deque<element_t *> *q = arg->q;
  char *val;
  bool passed;
  bool ret;
  cstore::nocomp_tuple_t nocomp_tuple;
  cstore::rle_triple_tuple_t rle_tuple;

  while (true) {
    // deque
    pthread_mutex_lock(&mutex);
    if (q->empty()) {
      //std::cout << "empty " << arg->id << std::endl;
      /*
      std::cout << "done_offset: " << done_offset << std::endl;
      std::cout << "done[0]: " << done[0] << ", done[1]: " << done[1] << ", done[2]: " << done[2] << std::endl;
      std::cout << "produced[0]: " << produced[0] << ", consumed[0]: " << consumed[0] << std::endl;
      std::cout << "produced[1]: " << produced[1] << ", consumed[1]: " << consumed[1] << std::endl;
      std::cout << "produced[2]: " << produced[2] << ", consumed[2]: " << consumed[2] << std::endl;
      */
      if (done_offset >= num_sensors + 2) {
        exit(0);
      }
      for (int i = done_offset; i < num_sensors + 2; ++i) {
        if (((i == 0 && index_traversed) || done[i-1]) && produced[i] == consumed[i]) {
          done[i] = true;
          done_offset = i + 1;
          if (done_offset == num_sensors + 2) {
            // all done
            exit(0);
          }
        } else {
          break;
        }
      }
      pthread_mutex_unlock(&mutex);
      usleep(100000);
      continue;
    }
    //std::cout << "NOT empty " << arg->id << std::endl;
    element_t *elem = q->front();
    q->pop_front();
    //element_t *elem = q->back();
    //q->pop_back();
    pthread_mutex_unlock(&mutex);
    /*
    if (elem->off >= 1) {
      std::cout << "off: " << elem->off << ", pos: " << elem->pos << std::endl;
    }
    */

    cstore::DS4 *ds4 = dsv.at(elem->off).at(arg->id);
    if (ds4->get_comp_type() == cstore::RLE) {
      ret = ds4->rle_op(elem->pos, &rle_tuple, &passed);
      //std::cout << "rle_op" << std::endl;
    } else {
      ret = ds4->nocomp_op(elem->pos, &nocomp_tuple, &passed);
      //std::cout << "nocomp_op" << std::endl;
    }

    if (using_index && elem->off == 0 && ds4->get_comp_type() == cstore::RLE) {
      for (off_t i = rle_tuple.start; i < rle_tuple.start + rle_tuple.length; ++i) {
        element_t *new_elem = new element_t();
        new_elem->off = elem->off + 1;
        new_elem->pos = i;

        pthread_mutex_lock(&mutex);
        q->push_back(new_elem);
        produced[new_elem->off]++;
        pthread_mutex_unlock(&mutex);
        //std::cout << "added 1" << std::endl;
      }
    } else if ((elem->off == 1 && passed) || elem->off != 1) {
      if (elem->off + 1 < num_sensors + 2) {
        element_t *new_elem = new element_t();
        new_elem->off = elem->off + 1;
        new_elem->pos = elem->pos;

        pthread_mutex_lock(&mutex);
        q->push_back(new_elem);
        produced[new_elem->off]++;
        pthread_mutex_unlock(&mutex);
        //std::cout << "added " << std::endl;
      }
    }

    pthread_mutex_lock(&mutex);
    consumed[elem->off]++;

    if (elem->off == 0) {
      if (using_index && index_traversed && produced[0] == consumed[0]) {
        done[0] = true;
        done_offset = 1;
      }
    } else if (done[elem->off-1] && produced[elem->off] == consumed[elem->off]) {
      done[elem->off] = true;
      done_offset = elem->off + 1;
    }

    /*
    for (int i = done_offset; i < num_sensors + 2; ++i) {
      if (i == 0) {
        if (using_index && index_traversed && produced[0] == consumed[0]) {
          done[0] = true;
          done_offset = 1;
          std::cout << "first done w/ index" << std::endl;
          std::cout.flush();
        }
      } else if (done[i-1] && produced[i] == consumed[i]) {
        done[i] = true;
        done_offset = i + 1;
        if (done_offset == num_sensors + 2) {
          // all done
          break;
          for (int j = 0; j < num_sensors + 2; ++j) {
            std::cout << "consumed[" << j << "]: " << consumed[j] << std::endl;
          }
          for (int j = 0; j < num_sensors + 2; ++j) {
            std::cout << "consumed[" << j << "]: " << consumed[j] << std::endl;
          }
          exit(0);
        }
      } else {
        break;
      }
    }
    */
    pthread_mutex_unlock(&mutex);

    delete(elem);
    // lookup
  }
  return NULL;
}
