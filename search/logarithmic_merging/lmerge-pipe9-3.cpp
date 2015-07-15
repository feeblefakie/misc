#include <iostream>
#include <fstream>
#include <vector>
#include <stdint.h>
#include <map>
#include <deque>
#include <string>
#include <luxio/btree.h>
#include <mecab.h>
#include <boost/algorithm/string.hpp>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define FLUSH_THRESHOLD 16*1024*1024
#define NUM_MEM_THREADS 8
#define NUM_FLUSH_BUFFER 8 // for buffers after in-memory inversion

double gettimeofday_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

//typedef std::vector<uint32_t> pos_list_t;
typedef std::deque<uint32_t> pos_list_t;
typedef std::vector<uint8_t> pos_list_enc_t;
typedef std::map<std::string, pos_list_t> pos_lists_tmp_t;
typedef std::map<std::string, pos_list_enc_t> pos_lists_t;
typedef pos_lists_t::iterator pos_lists_itr;
typedef std::map<std::string, Lux::IO::data_t *> data_lists_t;
typedef data_lists_t::iterator data_lists_itr;
typedef std::vector<std::string> str_list;
typedef std::vector<Lux::IO::data_t *> data_list;

int get_length_of(const unsigned char *str);
void LMergeAddToken(std::vector<bool> &indexes,
                    pos_lists_t &Z0,
                    std::pair<std::string, pos_list_t> &pos,
                    uint32_t &used_buffer_size);
void Z0_flush(std::vector<bool> &indexes);
void serialize_pos_list(pos_list_enc_t &list, char **p, size_t *size);
void dump(pos_lists_t &Z0, int i);
void merge(pos_lists_t &Z0, int i);
void merge0(pos_lists_t &Z0);
void mergei(int i);
void pipeline_merge(pos_lists_t &Z0, uint32_t level);
void pipeline_merge_linear(uint32_t level);
void *index_memory(void *p);
int get_global_seq_no(void);
void vb_encode_num(int n, std::vector<uint8_t> &bytestream);
std::vector<uint8_t> vb_encode(pos_list_t &numbers);
void push_or_merge(pos_lists_t *list);

pthread_mutex_t gsn_mutex;
pthread_mutex_t flush_mutex;
pthread_mutex_t flush_queue_mutex;
pthread_mutex_t flush_wait_mutex;
pthread_mutex_t flush_buffer_queue_mutex;
pthread_mutex_t fin_mutex;
pthread_cond_t flush_cond;
bool is_flush_done = false;
int flush_wait = 0;

//pos_lists_t Z0[NUM_MEM_THREADS];
std::vector<bool> indexes;
std::vector<int> flush_queue;
std::vector<pos_lists_t *> flush_buffer_queue;
int flush_buffer_queue_size = 0;

std::ifstream fin;

struct thread_arg_t {
  int id;
};

int global_seq_no_flushed = 0;

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << argv[0] << " file" << std::endl;
    exit(1);
  }

  pthread_mutex_init(&gsn_mutex, NULL);
  pthread_mutex_init(&flush_mutex, NULL);
  pthread_mutex_init(&flush_queue_mutex, NULL);
  pthread_mutex_init(&flush_wait_mutex, NULL);
  pthread_mutex_init(&flush_buffer_queue_mutex, NULL);
  pthread_mutex_init(&fin_mutex, NULL);
  pthread_cond_init(&flush_cond, NULL);

  fin.open(argv[1], std::ios::in);
  pthread_t tid[NUM_MEM_THREADS];
  thread_arg_t arg[NUM_MEM_THREADS];
  std::cout << "starting threads" << std::endl;
  for (int i = 0; i < NUM_MEM_THREADS; ++i) {

    arg[i].id = i;
    if (pthread_create(&tid[i], NULL, index_memory, (void *) &arg[i]) != 0) {
      perror("pthread_create");
      exit(1);
    }
  }

  void *ret = NULL;
  for (int i = 0; i < NUM_MEM_THREADS; ++i) {
    if (pthread_join(tid[i], &ret)) {
      perror("pthread_join");
    }
  }
  fin.close();

  return 0;
}

int get_global_seq_no(void)
{
  static int global_seq_no = 0;
  
  pthread_mutex_lock(&gsn_mutex);
  ++global_seq_no;
  pthread_mutex_unlock(&gsn_mutex);
  return global_seq_no;
}

void *index_memory(void *p)
{
  pos_lists_t *Z0 = new pos_lists_t;

  uint32_t used_buffer_size = 0;

  thread_arg_t *arg = (thread_arg_t *) p;
  int id = arg->id;
  double t1 = gettimeofday_sec();
  std::cout << "index_memory starts for Z0-" << id << std::endl;

  MeCab::Tagger *tagger = MeCab::createTagger(0, NULL);

  uint32_t doc_id = 1;
  std::string line;
  while (1) {
    pthread_mutex_lock(&fin_mutex);
    if (!getline(fin, line)) {
      pthread_mutex_unlock(&fin_mutex);
      break;
    }
    pthread_mutex_unlock(&fin_mutex);

    std::vector<std::string> items;
    boost::split(items, line, boost::is_any_of(""));

    /*
    if (items.size() != 6) { continue; }
    line = items[4] + " " + items[5];
    */
    if (items.size() != 3) { continue; }
    line = items[1] + " " + items[2];

    unsigned int offset = 0;
    pos_lists_tmp_t Ztmp;
    const MeCab::Node *node_ = tagger->parseToNode(line.c_str());
    for (; node_; node_ = node_->next) {
      if (node_->stat == MECAB_BOS_NODE ||
        node_->stat == MECAB_EOS_NODE) {
        continue; 
      }   
      std::string token(node_->surface, node_->length);
      unsigned int length = get_length_of((const unsigned char *) token.c_str());
      // mecab ignores spaces in default 
      // but they must be counted as offset from the beginning
      int head_space_len =  node_->rlength - node_->length;
      offset += head_space_len > 0 ? head_space_len : 0;

      pos_lists_tmp_t::iterator itr = Ztmp.find(token);
      if (itr != Ztmp.end()) {
        itr->second.push_back(offset);
        ++(itr->second.at(1));
      } else {
        pos_list_t p;
        p.push_back(doc_id);
        p.push_back(1);
        p.push_back(offset);
        std::pair<std::string, pos_list_t> pos;
        pos.first = token;
        pos.second = p;
        Ztmp.insert(pos);
      }
      //tokens_.push_back(Term(token, length, offset));
      offset += length;
    }

    pos_lists_tmp_t::iterator itr_end = Ztmp.end();
    for (pos_lists_tmp_t::iterator itr = Ztmp.begin();
         itr != itr_end; ++itr) {

      std::pair<std::string, pos_list_t> pos = *itr;
      LMergeAddToken(indexes, *Z0, pos, used_buffer_size);
    }
    //std::cout << "doc: " << doc_id << std::endl;
    ++doc_id;
    Ztmp.clear();
    pos_lists_tmp_t().swap(Ztmp);

    if (used_buffer_size > FLUSH_THRESHOLD) {
      pthread_mutex_lock(&flush_buffer_queue_mutex);
      ++flush_buffer_queue_size;
      push_or_merge(Z0);
      //flush_buffer_queue.push_back(Z0);
      if (flush_buffer_queue_size == NUM_FLUSH_BUFFER) {
        // flushing (Z0 is cleard inside Z0_flush)
        Z0_flush(indexes);
        flush_buffer_queue_size = 0;
      }
      Z0 = new pos_lists_t;
      pthread_mutex_unlock(&flush_buffer_queue_mutex);
      used_buffer_size = 0;
      // [TODO] clear doc_id
    }
  }
  if (used_buffer_size > 0) {
    pthread_mutex_lock(&flush_buffer_queue_mutex);
    //flush_buffer_queue.push_back(Z0);
    ++flush_buffer_queue_size;
    push_or_merge(Z0);
    if (flush_buffer_queue_size == NUM_FLUSH_BUFFER) {
      Z0_flush(indexes);
    }
    pthread_mutex_unlock(&flush_buffer_queue_mutex);
  }
  delete tagger;
  double t2 = gettimeofday_sec();
  std::cout << "index_memory ends for Z0-" << id << " - time: " << t2 -t1 << std::endl;

  return NULL;
}

void push_or_merge(pos_lists_t *list)
{
  if (flush_buffer_queue.empty()) {
    // just pushing
    flush_buffer_queue.push_back(list);
  } else {
    // merging with existing memory index
    pos_lists_t *base = flush_buffer_queue[0];
    pos_lists_itr itr_base = base->begin();
    pos_lists_itr itr = list->begin();
    pos_lists_itr itr_end = list->end();
    bool is_base_forwarded = false;

    // 2-way merge
    while (itr_base != base->end() && itr != itr_end) {
      if (itr_base == base->end()) {
        base->insert(itr_base, pos_lists_t::value_type(itr->first, itr->second));
        ++itr;
        continue;
      }
      if (itr == itr_end) {
        ++itr_base;
        continue;
      }

      if (itr_base->first == itr->first) {
        //itr_base->second += itr->second;
        pos_list_enc_t::iterator tmp_itr = itr->second.begin();
        pos_list_enc_t::iterator tmp_itr_end = itr->second.end();
        for (; tmp_itr != tmp_itr_end; ++tmp_itr) {
          itr_base->second.push_back(*tmp_itr);
        }
        ++itr_base;
        ++itr;
      } else if (itr_base->first < itr->first) {
        ++itr_base;
      } else {
        if (itr_base != base->begin()) {
          advance(itr_base, -1);
          itr_base = base->insert(itr_base, pos_lists_t::value_type(itr->first, itr->second));
        } else {
          itr_base = base->insert(itr_base, pos_lists_t::value_type(itr->first, itr->second));
        }
        ++itr_base;
        ++itr;
      }
    }
    list->clear();
    delete list;
  }
}

int get_length_of(const unsigned char *str)
{   
  size_t str_len = std::strlen((char *) str);
  int ustr_len = 0;

  for (int i = 0; i < str_len; i++) {
    if (str[i] <= 0x7f || (str[i] >= 0xc0 && str[i] <= 0xfd)) {
      ustr_len++;
    }   
  }   
  return ustr_len;
}

void LMergeAddToken(std::vector<bool> &indexes,
                    pos_lists_t &Z0,
                    std::pair<std::string, pos_list_t> &pos,
                    uint32_t &used_buffer_size)
{
  //std::cout << pos.first << std::endl;
  pos_lists_t::iterator itr = Z0.find(pos.first);
  if (itr != Z0.end()) {
    /*
    pos_list_t::iterator pitr_end = pos.second.end();
    for (pos_list_t::iterator pitr = pos.second.begin(); pitr != pitr_end; ++pitr) {
      itr->second.push_back(*pitr);
      used_buffer_size += sizeof(uint32_t);
    }
    */
    std::vector<uint8_t> encoded_pos = vb_encode(pos.second);
    std::vector<uint8_t>::iterator eitr_end = encoded_pos.end();
    for (std::vector<uint8_t>::iterator eitr = encoded_pos.begin(); eitr != eitr_end; ++eitr) {
      itr->second.push_back(*eitr);
    }
    used_buffer_size += sizeof(uint8_t) * encoded_pos.size();
  } else {
    //Z0.insert(pos);
    //used_buffer_size += sizeof(char) * pos.first.size();
    //used_buffer_size += sizeof(uint32_t) * pos.second.size();

    std::vector<uint8_t> encoded_pos = vb_encode(pos.second);
    Z0.insert(pos_lists_t::value_type(pos.first, encoded_pos));
    used_buffer_size += sizeof(char) * pos.first.size();
    used_buffer_size += sizeof(uint8_t) * encoded_pos.size();
  }
}

void Z0_flush(std::vector<bool> &indexes)
{
  std::cout << "Z0_flush" << std::endl;

  double t1 = gettimeofday_sec();

  int i = 0;
  if (indexes.empty()) {
    indexes.push_back(false);
  }
  while (1) {
    // pipelining test
    uint32_t pipeline_level = 0;
    for (int j = 0; j < indexes.size(); ++j) {
      if (indexes[j]) {
        ++pipeline_level;
      } else {
        break;
      }
    }

    pipeline_merge_linear(pipeline_level);
    for (int k = 0; k < pipeline_level; ++k) {
      indexes[k] = false;
      char fname1[256];
      char fname2[256];
      sprintf(fname1, "I%d.bidx", k);
      sprintf(fname2, "I%d.data", k);
      if (unlink(fname1) < 0) { perror(fname1); }
      if (unlink(fname2) < 0) { perror(fname2); }
    }
    if (indexes.size() != pipeline_level + 1) {
      indexes.push_back(false);
    }
    indexes[pipeline_level] = true;
    break;
    
    /*
    if (indexes[i]) {
      assert(0);
      merge(Z0, i);
      indexes[i] = false;
      // [NOTICE] they shouldn't be removed here.
      char fname1[256];
      char fname2[256];
      sprintf(fname1, "I%d.bidx", i);
      sprintf(fname2, "I%d.data", i);
      if (unlink(fname1) < 0) { perror("unlink failed"); }
      if (unlink(fname2) < 0) { perror("unlink failed"); }
    } else {
      dump(Z0, i);
      indexes[i] = true;
      break;
    }
    assert(0);
    ++i;
    indexes.push_back(false);
    */
  }
  std::vector<pos_lists_t *>::iterator itr_end = flush_buffer_queue.end();
  for (std::vector<pos_lists_t *>::iterator itr = flush_buffer_queue.begin(); itr != itr_end; ++itr) {
    (*itr)->clear();
    delete *itr;
  }
  flush_buffer_queue.clear();
  double t2 = gettimeofday_sec();
  std::cout << "flush(merge+I/O) time: " << t2 - t1 << std::endl;
}

void pipeline_merge_linear(uint32_t level)
{
  std::cerr << "pipelining merge - level: " << level << std::endl;
  double t1 = gettimeofday_sec();

  int num_flush_buffer = flush_buffer_queue.size();

  // couter[0] and cursor_finished[0] for Z0
  int counter[level+num_flush_buffer];
  bool cursor_finished[level+num_flush_buffer];
  for (int i = 0; i < level + num_flush_buffer; ++i) {
    counter[i] = 0;
    cursor_finished[i] = false;
  }

  std::vector<Lux::IO::Btree *> bts;
  std::vector<Lux::IO::cursor_t *> btc;
  //str_list current_key;
  std::string current_key[level+num_flush_buffer];
  //data_list current_data;
  Lux::IO::data_t *current_data[level+num_flush_buffer];
  //current_key.reserve(level+1);
  //current_data.reserve(level+1);

  for (int i = num_flush_buffer; i < level + num_flush_buffer; ++i) {
    Lux::IO::Btree *bt = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
    char fname[256];
    memset(fname, 0, 256);
    sprintf(fname, "I%d", i-num_flush_buffer);
    if (!bt->open(fname, Lux::IO::DB_RDONLY)) {
      std::cerr << "open error for " << fname << std::endl;
      perror(fname);
      exit(1);
    }
    Lux::IO::cursor_t *c = bt->cursor_init();
    if (!bt->first(c)) {
      std::cerr << "cursor first failed" << std::endl;
      cursor_finished[i] = true;
    } else {
      Lux::IO::data_t *key;
      Lux::IO::data_t *val;
      if (!bt->cursor_get(c, &key, &val, Lux::IO::SYSTEM)) {
        std::cerr << "cursor_get failed" << std::endl;
      } else {
        current_key[i] = std::string((char *) key->data, key->size);
        current_data[i] = val;
        bt->clean_data(key);
      }
    }
    bts.push_back(bt);
    btc.push_back(c);
  }

  // create I(level+1)
  Lux::IO::Btree *btI = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
  btI->set_noncluster_params(Lux::IO::Padded, Lux::IO::NOPADDING);
  char fname[256];
  memset(fname, 0, 256);
  sprintf(fname, "I%d", level);
  if (!btI->open(fname, Lux::IO::DB_CREAT)) {
    std::cerr << "open error for " << fname << std::endl;
    perror(fname);
  }
  btI->set_bulk_loading(true);

// tmporary
#define Z0NEXT(idx) \
  if (itrs[idx] == itr_ends[idx]) { \
    cursor_finished[idx] = true; \
  } else { \
    current_key[idx] = itrs[idx]->first; \
    char *p; \
    size_t val_size; \
    serialize_pos_list(itrs[idx]->second, &p, &val_size); \
    Lux::IO::data_t *dt = new Lux::IO::data_t; \
    dt->data = p; \
    dt->size = val_size; \
    current_data[idx] = dt; \
  }

#define IiNEXT(idx) \
  if (!bts[idx-num_flush_buffer]->next(btc[idx-num_flush_buffer])) { \
    cursor_finished[idx] = true; \
  } else { \
    Lux::IO::data_t *key; \
    Lux::IO::data_t *val; \
    if (!bts[idx-num_flush_buffer]->cursor_get(btc[idx-num_flush_buffer], &key, &val, Lux::IO::SYSTEM)) { \
      std::cerr << "cursor_get failed" << std::endl; \
    } \
    current_key[idx] = std::string((char *) key->data, key->size); \
    current_data[idx] = val; \
    bts[idx-num_flush_buffer]->clean_data(key); \
  }

  // Z0 iterator
  std::vector<pos_lists_itr> itrs;
  std::vector<pos_lists_itr> itr_ends;
  for (int i = 0; i < num_flush_buffer; ++i) {
    itrs.push_back(flush_buffer_queue[i]->begin());
    itr_ends.push_back(flush_buffer_queue[i]->end());
    Z0NEXT(i);
  }

  while (1) {
    bool is_all_done = true;
    for (int i = 0; i < level + num_flush_buffer; ++i) {
      if (!cursor_finished[i]) {
        is_all_done = false;
        break;
      }   
    }   
    if (is_all_done) { break; }
    
    std::string smallest_key;
    std::vector<int> smallest_i;
    int num_smallest = 0;

    bool is_set = false;
    for (int i = 0; i < level + num_flush_buffer; ++i) {
      if (cursor_finished[i]) { continue; }
      if (!is_set) {
        // set first entry
        smallest_key = current_key[i];
        smallest_i.push_back(i);
        is_set = true;
        num_smallest = 1;
      } else {
        if (current_key[i] < smallest_key) {
          smallest_key = current_key[i];
          smallest_i.push_back(i);
          num_smallest = 1;
        } else if (current_key[i] == smallest_key) {
          smallest_i.push_back(i);
          ++num_smallest;
        }
      }
    }

    // comparison for smallest
    if (is_set) {
      // num_smallest from the end
      if (num_smallest > 1) {
        // combine
        size_t size = smallest_i.size();
        size_t total = 0;
        for (int i = size - num_smallest; i < size; ++i) {
          int idx = smallest_i[i];
          total += current_data[idx]->size;
        }
        Lux::IO::data_t new_data;
        new_data.data = new char[total];
        new_data.size = total;
        char *p = (char *) new_data.data;
        for (int i = size - num_smallest; i < size; ++i) {
          int idx = smallest_i[i];
          memcpy(p, current_data[idx]->data, current_data[idx]->size);
          p += current_data[idx]->size;
          if (idx < num_flush_buffer) {
            delete current_data[idx];
          } else {
            btI->clean_data(current_data[idx]);
          }
          if (idx < num_flush_buffer) {
            ++itrs[idx];
            Z0NEXT(idx);
          } else {
            IiNEXT(idx);
          }
        }

        Lux::IO::data_t key = {smallest_key.c_str(), smallest_key.length()};
        Lux::IO::data_t *val = &new_data;
        btI->put(&key, val);
        delete [] (char *) (new_data.data);

      } else {
        // output the key and value
        int idx = smallest_i[smallest_i.size()-1];
        Lux::IO::data_t key = {smallest_key.c_str(), smallest_key.length()};
        Lux::IO::data_t *val = current_data[idx];
        btI->put(&key, val);
        if (idx < num_flush_buffer) {
          delete val;
        } else {
          btI->clean_data(val);
        }

        // forward the cursor to the next
        if (idx < num_flush_buffer) { // Z0
          ++itrs[idx];
          Z0NEXT(idx);
        } else {
          IiNEXT(idx);
        }
      }
    }
  }

  for (int i = 0; i < bts.size(); ++i) {
    bts[i]->cursor_fin(btc[i]);
    bts[i]->close();
    delete bts[i];
  }

  if (!btI->close()) {
    perror("btI");
  }
  delete btI;

  double t2 = gettimeofday_sec();
  std::cerr << level+1 << "-way merge: " << t2 - t1 << " (s)" << std::endl;
}

void pipeline_merge(pos_lists_t &Z0, uint32_t level)
{
  std::cerr << "pipelining merge - level: " << level << std::endl;
  double t1 = gettimeofday_sec();
  std::cout << "Z0 size" << Z0.size() << std::endl;

  int counter[level];
  bool cursor_finished[level];
  for (int i = 0; i < level; ++i) {
    counter[i] = 0;
    cursor_finished[i] = false;
  }

  // open index I1 - Ilevel
  std::vector<Lux::IO::Btree *> bts;
  std::vector<Lux::IO::cursor_t *> btc;

  for (int i = 0; i < level; ++i) {
    Lux::IO::Btree *bt = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
    char fname[256];
    memset(fname, 0, 256);
    sprintf(fname, "I%d", i);
    if (!bt->open(fname, Lux::IO::DB_RDONLY)) {
      std::cerr << "open error for " << fname << std::endl;
      perror(fname);
    }
    Lux::IO::cursor_t *c = bt->cursor_init();
    if (!bt->first(c)) {
      std::cerr << "cursor first failed" << std::endl;
      cursor_finished[i] = true;
    }
    bts.push_back(bt);
    btc.push_back(c);

    std::cout << "I" << i << " size" << std::endl;
    bt->show_db_header();
  }

  // create I(level+1)
  Lux::IO::Btree *btI = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
  btI->set_noncluster_params(Lux::IO::Padded, Lux::IO::NOPADDING);
  char fname[256];
  memset(fname, 0, 256);
  sprintf(fname, "I%d", level);
  if (!btI->open(fname, Lux::IO::DB_CREAT)) {
    std::cerr << "open error for " << fname << std::endl;
    perror(fname);
  }
  btI->set_bulk_loading(true);

  // pipiline proceeded by the specified number
  uint32_t pipeline_by_num = 10;
  pos_lists_itr itr_end = Z0.end();
  for (pos_lists_itr itr = Z0.begin(); itr != itr_end; ++itr) {

    data_lists_t dlist;
    char *base_str;
    int k = 0;
    do {
      std::pair<std::string, Lux::IO::data_t *> list;
      list.first = itr->first;
      char *p;
      size_t val_size;
      serialize_pos_list(itr->second, &p, &val_size);
      // FIXME: this must be allocated in heap (free store)
      Lux::IO::data_t *v = new Lux::IO::data_t;
      v->data = p;
      v->size = val_size;
      list.second = v;
      dlist.insert(list);
      base_str = (char *) itr->first.c_str();
      ++itr;
      ++k;
    } while ((itr != itr_end) && (k < pipeline_by_num));
    --itr;

    //char *base_str = (char *) itr->first.c_str();
    for (int i = 0; i < level; ++i) {
      Lux::IO::data_t *key;
      Lux::IO::data_t *val;
      while (!cursor_finished[i]) {
        if (!bts[i]->cursor_get(btc[i], &key, &val, Lux::IO::SYSTEM)) {
          std::cerr << "cursor_get failed" << std::endl;
        }
        int n = strcmp(base_str, (char *) key->data);
        if (n < 0) {
          bts[i]->clean_data(key);
          bts[i]->clean_data(val);
          break;
        } else {
          // add
          std::pair<std::string, Lux::IO::data_t *> list;
          list.first = std::string((char *) key->data);
          list.second = val;

          data_lists_t::iterator d_itr = dlist.find(list.first);
          if (d_itr != dlist.end()) {
            // concatenate
            uint32_t newsize = d_itr->second->size + val->size;
            char *newlist = new char[newsize];
            char *p = newlist;
            memcpy(p, d_itr->second->data, d_itr->second->size);
            memcpy(p + d_itr->second->size, val->data, val->size);
            Lux::IO::data_t *newval = new Lux::IO::data_t;
            newval->data = newlist;
            newval->size = newsize;
            // old second must be deleted (the pointer and the pointing data also)
            delete [] (char *) (d_itr->second->data);
            delete d_itr->second;
            d_itr->second = newval;

            // deleting in this case only
            bts[i]->clean_data(val);
          } else {
            // just add
            dlist.insert(list);
          }
          bts[i]->clean_data(key);
        }
        ++counter[i];
        if (!bts[i]->next(btc[i])) {
          cursor_finished[i] = true;
        }
      }
    }

    // write to I(level+1)
    data_lists_itr d_itr_end = dlist.end();
    for (data_lists_itr d_itr = dlist.begin(); d_itr != d_itr_end; ++d_itr) {
      Lux::IO::data_t key = {d_itr->first.c_str(), d_itr->first.length()};
      Lux::IO::data_t *val = d_itr->second;
      btI->put(&key, val);
      delete [] (char *) (d_itr->second->data);
      delete d_itr->second;
    }
    dlist.clear();
  }

  /* TODO: when Z0 list is finished first, then remaining lists from another Ii is not indexed. */
  // FIXME: bad code. basic procedure is very similar to the above one. better to integrate.
  // Ii is the base
  for (int i = 0; i < level; ++i) {
    while (!cursor_finished[i]) {
      Lux::IO::data_t *key;
      Lux::IO::data_t *val;
      if (!bts[i]->cursor_get(btc[i], &key, &val, Lux::IO::SYSTEM)) {
        std::cerr << "cursor_get failed" << std::endl;
      }

      data_lists_t dlist;
      std::pair<std::string, Lux::IO::data_t *> list;
      list.first = std::string((char *) key->data);
      list.second = val;
      dlist.insert(list);

      char *base_str = (char *) key->data;
      for (int j = i + 1; j < level; ++j) {

        while (!cursor_finished[j]) {
          Lux::IO::data_t *k;
          Lux::IO::data_t *v;
          if (!bts[j]->cursor_get(btc[j], &k, &v, Lux::IO::SYSTEM)) {
            std::cerr << "cursor_get failed" << std::endl;
          }
          int n = strcmp(base_str, (char *) k->data);
          if (n < 0) {
            break;
          } else {
            // add
            std::pair<std::string, Lux::IO::data_t *> list;
            list.first = std::string((char *) k->data);
            list.second = v;

            data_lists_t::iterator d_itr = dlist.find(list.first);
            if (d_itr != dlist.end()) {
              // concatenate
              uint32_t newsize = d_itr->second->size + v->size;
              char *newlist = new char[newsize];
              char *p = newlist;
              memcpy(p, d_itr->second->data, d_itr->second->size);
              memcpy(p + d_itr->second->size, v->data, v->size);
              Lux::IO::data_t *newval = new Lux::IO::data_t;
              newval->data = newlist;
              newval->size = newsize;
              d_itr->second = newval;
            } else {
              // just add
              dlist.insert(list);
            }
          }
          if (!bts[j]->next(btc[j])) {
            cursor_finished[j] = true;
          }
        }
      }
      // write to I(level+1)
      data_lists_itr d_itr_end = dlist.end();
      for (data_lists_itr d_itr = dlist.begin(); d_itr != d_itr_end; ++d_itr) {
        Lux::IO::data_t key = {d_itr->first.c_str(), d_itr->first.length()};
        Lux::IO::data_t *val = d_itr->second;
        btI->put(&key, val);
      }
      dlist.clear();

      if (!bts[i]->next(btc[i])) {
        cursor_finished[i] = true;
      }
    }
  }

  for (int i = 0; i < level; ++i) {
    if (cursor_finished[i]) {
      std::cout << "true ";
    } else {
      std::cout << "false ";
    }
  }
  std::cout << std::endl;

  for (int i = 0; i < bts.size(); ++i) {
    bts[i]->cursor_fin(btc[i]);
    bts[i]->close();
    delete bts[i];
  }

  if (!btI->close()) {
    perror("btI");
  }
  delete btI;
  double t2 = gettimeofday_sec();
  std::cerr << level+1 << "-way merge: " << t2 - t1 << " (s)" << std::endl;
}

void merge(pos_lists_t &Z0, int i)
{
  if (i == 0) {
    // merge I0 with memory buffer Z0
    merge0(Z0);

  } else {
    // merge Ii with temporary index Zi
    mergei(i);
  }

}

void merge0(pos_lists_t &Z0)
{
  double t1 = gettimeofday_sec();
  Lux::IO::Btree *btZ1 = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
  btZ1->set_noncluster_params(Lux::IO::Padded, Lux::IO::NOPADDING);
  if (!btZ1->open("Z1", Lux::IO::DB_CREAT)) {
    std::cerr << "open error for Z1" << std::endl;
    perror("Z1");
  }
  btZ1->set_bulk_loading(true);

  Lux::IO::Btree *btI0 = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
  if (!btI0->open("I0", Lux::IO::DB_RDONLY)) {
    std::cerr << "open error for I0" << std::endl;
    perror("I0");
  }

  pos_lists_itr itr = Z0.begin();
  pos_lists_itr itr_end = Z0.end();
  Lux::IO::cursor_t *c = btI0->cursor_init();
  if (!btI0->first(c)) {
    std::cerr << "cursor first failed" << std::endl;
  }
  int i = 0;
  while (1) {
    bool is_I0_all_read = false;
    bool is_Z0_all_read = false;
    Lux::IO::data_t *key;
    Lux::IO::data_t *val;
    if (!btI0->cursor_get(c, &key, &val, Lux::IO::SYSTEM)) {
      std::cerr << "cursor_get failed" << std::endl;
    }

    int n = strncmp(itr->first.c_str(), (char *) key->data, key->size);
    if (n > 0) {
      /*
      std::cout << "write I0: ";
      std::cout.write((char *) key->data, key->size);
      std::cout << std::endl;
      */
      // put disk side
      btZ1->put(key, val);
      if (!btI0->next(c)) { is_I0_all_read = true; }
    } else if (n == 0) {
      /*
      std::cout << "write I0+Z0 : ";
      std::cout.write((char *) key->data, key->size);
      std::cout << std::endl;
      */
      // combine disk + memory
      char *p;
      size_t val_size;
      serialize_pos_list(itr->second, &p, &val_size);
      size_t new_val_size = val->size + val_size;
      char *new_val = new char[new_val_size];
      memcpy(new_val, val->data, val->size);
      memcpy(new_val + val->size, p, val_size);

      Lux::IO::data_t key = {itr->first.c_str(), itr->first.length()};
      Lux::IO::data_t val = {new_val, new_val_size};
      btZ1->put(&key, &val);
      //delete [] p;
      delete [] new_val;

      if (!btI0->next(c)) { is_I0_all_read = true; }
      if (++itr == itr_end) { is_Z0_all_read = true; }

    } else {
      /*
      std::cout << "write Z0 : ";
      std::cout << itr->first;
      std::cout << std::endl;
      */
      // put memory side
      char *p;
      size_t val_size;
      serialize_pos_list(itr->second, &p, &val_size);

      Lux::IO::data_t key = {itr->first.c_str(), itr->first.length()};
      Lux::IO::data_t val = {p, val_size};
      btZ1->put(&key, &val);
      delete [] p;

      if (++itr == itr_end) { is_Z0_all_read = true; }
    }
    ++i;
    btI0->clean_data(key);
    btI0->clean_data(val);

    // if either list is read up
    if (is_I0_all_read) {
      while (itr != itr_end) {
        char *p;
        size_t val_size;
        serialize_pos_list(itr->second, &p, &val_size);

        Lux::IO::data_t key = {itr->first.c_str(), itr->first.length()};
        Lux::IO::data_t val = {p, val_size};
        btZ1->put(&key, &val);
        //std::cout << "Z0 flush: " << itr->first << std::endl;
        delete [] p;
        ++itr;
      }
      break;
    } else if (is_Z0_all_read) {
      do {
        Lux::IO::data_t *key;
        Lux::IO::data_t *val;
        if (!btI0->cursor_get(c, &key, &val, Lux::IO::SYSTEM)) {
          std::cerr << "cursor_get failed" << std::endl;
        }
        btZ1->put(key, val);
        /*
        std::cout << "I0 flush: ";
        std::cout.write((char *) key->data, key->size);
        std::cout << std::endl;
        */

        btI0->clean_data(key);
        btI0->clean_data(val);
      } while (btI0->next(c));
      break;
    }
  }

  btI0->cursor_fin(c);
  if (!btI0->close()) {
    std::cerr << "close failed" << std::endl;
  }
  if (!btZ1->close()) {
    std::cerr << "close failed" << std::endl;
  }

  delete btI0;
  delete btZ1;
  double t2 = gettimeofday_sec();
  std::cerr << "2-way merge (I0 and Z0): " << t2 - t1 << " (s)" << std::endl;
}

void mergei(int i)
{
  double t1 = gettimeofday_sec();
  // merge Ii with temporary index Zi
  Lux::IO::Btree *btZi1 = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
  btZi1->set_noncluster_params(Lux::IO::Padded, Lux::IO::NOPADDING);
  char fname[256];
  sprintf(fname, "Z%d", i+1);
  if (!btZi1->open(fname, Lux::IO::DB_CREAT)) {
    std::cerr << "open error for " << fname << std::endl;
    perror("Zi1");
  }
  btZi1->set_bulk_loading(true);

  memset(fname, 0, 256);
  sprintf(fname, "Z%d", i);
  Lux::IO::Btree *btZi = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
  if (!btZi->open(fname, Lux::IO::DB_RDONLY)) {
    std::cerr << "open error for " << fname << std::endl;
    perror("Zi");
  }
  memset(fname, 0, 256);
  sprintf(fname, "I%d", i);
  Lux::IO::Btree *btIi = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
  if (!btIi->open(fname, Lux::IO::DB_RDONLY)) {
    std::cerr << "open error for " << fname << std::endl;
    perror("Ii");
  }

  Lux::IO::cursor_t *cI = btIi->cursor_init();
  Lux::IO::cursor_t *cZ = btZi->cursor_init();
  if (!btIi->first(cI)) {
    std::cerr << "first cursor error for btIi" << std::endl;
  }
  if (!btZi->first(cZ)) {
    std::cerr << "first cursor error for btZi" << std::endl;
  }
  int j = 0;
  while (1) {
    bool is_Ii_all_read = false;
    bool is_Zi_all_read = false;

    Lux::IO::data_t *keyI;
    Lux::IO::data_t *valI;
    if (!btIi->cursor_get(cI, &keyI, &valI, Lux::IO::SYSTEM)) {
      std::cerr << "cursor_get failed" << std::endl;
    }
    Lux::IO::data_t *keyZ;
    Lux::IO::data_t *valZ;
    if (!btZi->cursor_get(cZ, &keyZ, &valZ, Lux::IO::SYSTEM)) {
      std::cerr << "cursor_get failed" << std::endl;
    }

    int n = strncmp((char *) keyI->data, (char *) keyZ->data, keyI->size > keyZ->size ? keyZ->size : keyI->size);
    if (n > 0 || (n == 0 && keyI->size > keyZ->size)) {
      // put disk side
      btZi1->put(keyZ, valZ);
      if (!btZi->next(cZ)) { is_Zi_all_read = true; }
    } else if (n < 0 || (n == 0 && keyZ->size > keyI->size)) {
      // put disk side
      btZi1->put(keyI, valI);
      if (!btIi->next(cI)) { is_Ii_all_read = true; }
    } else {
      size_t new_val_size = valI->size + valZ->size;
      char *new_val = new char[new_val_size];
      memcpy(new_val, valI->data, valI->size);
      memcpy(new_val + valI->size, valZ->data, valZ->size);

      Lux::IO::data_t val = {new_val, new_val_size};
      btZi1->put(keyI, &val);
      delete [] new_val;
      if (!btIi->next(cI)) { is_Ii_all_read = true; }
      if (!btZi->next(cZ)) { is_Zi_all_read = true; }
    }
    btIi->clean_data(keyI);
    btIi->clean_data(valI);
    btZi->clean_data(keyZ);
    btZi->clean_data(valZ);

    if (is_Zi_all_read && !is_Ii_all_read) {
      do {
        Lux::IO::data_t *key;
        Lux::IO::data_t *val;
        if (!btIi->cursor_get(cI, &key, &val, Lux::IO::SYSTEM)) {
          std::cerr << "cursor_get failed" << std::endl;
        }
        btZi1->put(key, val);
        /*
        std::cout << "Ii flush remainings: ";
        std::cout.write((char *) key->data, key->size);
        std::cout << std::endl;
        */
        btIi->clean_data(key);
        btIi->clean_data(val);
      } while (btIi->next(cI));
      break;
    } else if (is_Ii_all_read && !is_Zi_all_read) {
      do {
        Lux::IO::data_t *key;
        Lux::IO::data_t *val;
        if (!btZi->cursor_get(cZ, &key, &val, Lux::IO::SYSTEM)) {
          std::cerr << "cursor_get failed" << std::endl;
        }
        btZi1->put(key, val);
        /*
        std::cout << "Zi flush remainings: ";
        std::cout.write((char *) key->data, key->size);
        std::cout << std::endl;
        */
        btZi->clean_data(key);
        btZi->clean_data(val);
      } while (btZi->next(cZ));
      break;
    } else if (is_Ii_all_read && is_Zi_all_read) {
        break;
    }
    ++j;
    //std::cout << j << std::endl;
  }

  btIi->cursor_fin(cI);
  btZi->cursor_fin(cZ);
  if (!btIi->close()) {
    std::cerr << "close failed" << std::endl;
  }
  delete btIi;
  if (!btZi->close()) {
    std::cerr << "close failed" << std::endl;
  }
  delete btZi;
  if (!btZi1->close()) {
    std::cerr << "close failed" << std::endl;
  }
  delete btZi1;

  char fname1[256];
  char fname2[256];
  sprintf(fname1, "Z%d.bidx", i);
  sprintf(fname2, "Z%d.data", i);
  if (unlink(fname1) < 0) { perror("unlink failed"); }
  if (unlink(fname2) < 0) { perror("unlink failed"); }
  double t2 = gettimeofday_sec();
  std::cerr << "2-way merge (I" << i << " and Z" << i << "): " << t2 - t1 << " (s)" << std::endl;
}

void dump(pos_lists_t &Z0, int i)
{
  //std::cout << "dump!" << std::endl;
  if (i == 0) {
    Lux::IO::Btree *bt = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
    bt->set_noncluster_params(Lux::IO::Padded, Lux::IO::NOPADDING);
    if (!bt->open("I0", Lux::IO::DB_CREAT)) {
      std::cerr << "open error for I0" << std::endl;
      perror("I0");
    }
    bt->set_bulk_loading(true);

    // dump Z0 with name I0
    pos_lists_itr itr_end = Z0.end();
    for (pos_lists_itr itr = Z0.begin(); itr != itr_end; ++itr) {
      char *p;
      size_t val_size;
      serialize_pos_list(itr->second, &p, &val_size);

      Lux::IO::data_t key = {itr->first.c_str(), itr->first.length()};
      Lux::IO::data_t val = {p, val_size};
      bt->put(&key, &val);
      //delete [] p;
    }
    if (!bt->close()) {
      std::cerr << "close failed" << std::endl;
    }
    delete bt;

  } else {
    // rename Zi to Ii
    char fname_old[256];
    char fname_new[256];

    sprintf(fname_old, "Z%d.bidx", i);
    sprintf(fname_new, "I%d.bidx", i);
    if (rename(fname_old, fname_new) < 0) {
      std::cerr << "rename failed" << std::endl;
    }
    memset(fname_old, 0, 255);
    memset(fname_new, 0, 255);
    sprintf(fname_old, "Z%d.data", i);
    sprintf(fname_new, "I%d.data", i);
    if (rename(fname_old, fname_new) < 0) {
      std::cerr << "rename failed" << std::endl;
    }
  }

}

inline
void serialize_pos_list(pos_list_enc_t &list, char **p, size_t *size)
{
  /*
  *size = list.size() * sizeof(uint32_t);
  *p = new char[*size];
  char *buf = *p;
  pos_list_t::iterator itr_end = list.end();
  for (pos_list_t::iterator itr = list.begin(); itr != itr_end; ++itr) {
    memcpy(buf, &(*itr), sizeof(uint32_t));
    buf += sizeof(uint32_t);
  }
  */
  *p = (char *) &(list[0]);
  *size = list.size();
}

inline
std::vector<uint8_t> vb_encode(pos_list_t &numbers)
{
  std::vector<uint8_t> bytestream;
  //std::vector<uint32_t>::iterator itr = numbers.begin();
  pos_list_t::iterator itr = numbers.begin();
  //std::vector<uint32_t>::iterator itr_end = numbers.end();
  pos_list_t::iterator itr_end = numbers.end();
  int prev_doc_id = 0;
  int doc_id_gap;
  int num_postings = 0;

  int i = 0;
  for (; itr != itr_end; ++itr) {
    if (num_postings == 0) {
      ++i;
      doc_id_gap = *itr - prev_doc_id;
      prev_doc_id = *itr;
      vb_encode_num(doc_id_gap, bytestream);
      num_postings = *(++itr);
      vb_encode_num(num_postings, bytestream);
    } else {
      int prev_pos = 0;
      while (itr != itr_end) {
        vb_encode_num(*itr-prev_pos, bytestream);
        prev_pos = *itr;
        if (--num_postings > 0) {
          ++itr;
        } else {
          break;
        }
      }
    }
  }
  return bytestream;
}

inline
void vb_encode_num(int n, std::vector<uint8_t> &bytestream)
{
  std::vector<uint8_t> bytes;
  bytes.reserve(8);
  while (true) {
    unsigned char rem = n % 128;
    bytes.push_back(rem);
    if (n < 128) break;
    n /= 128;
  }
  bytes[0] += 128;
  size_t size = bytes.size();
  for (int i = size - 1; i >= 0; --i) {
    bytestream.push_back(bytes[i]); 
  }
}
