#ifndef COLUMN_H
#define COLUMN_H

//#define HAVE_CXX_STDHEADERS

#include <db_cxx.h>
#include <cstring>
#include <deque>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "util.h"

#define DEFAULT_BUFSIZE 4096

namespace cstore {

  typedef enum {
    NOCOMP,
    RLE
  } comp_t;

  typedef enum {
    TINYINT,
    INT,
    BIGINT,
    CHAR,
    CHAR4,
    CHAR8,
    DOUBLE
  } col_t;

  typedef enum {
    EQ,
    NE,
    GT,
    GE,
    LT,
    LE
  } compare_t;

  typedef struct {
    comp_t comp_type;
    col_t col_type;
    off_t offset;
    size_t val_size;
    size_t tuple_size;
    off_t n_tuples;
    bool indexed;
  } column_header_t;

#pragma pack(1)
  typedef struct {
    char val[1];
    uint64_t start;
    uint64_t length;
  } rle_triple_val1_t;
#pragma pack()

#pragma pack(1)
  typedef struct {
    char val[4];
    uint64_t start;
    uint64_t length;
  } rle_triple_val4_t;
#pragma pack()

  typedef struct {
    char val[8];
    uint64_t start;
    uint64_t length;
  } rle_triple_val8_t;

  typedef struct {
    char val[8];
    uint64_t start;
    uint64_t length;
  } rle_triple_tuple_t;

#pragma pack(1)
  typedef struct {
    char val[1];
  } nocomp_val1_t;
#pragma pack()

  typedef struct {
    char val[4];
  } nocomp_val4_t;

  typedef struct {
    char val[8];
  } nocomp_val8_t;

  typedef struct {
    char val[8];
  } nocomp_tuple_t;

  class Column {
  public:
    Column();
    ~Column();
    bool open(std::string col_name, col_flags_t oflags, bool indexed = false); 
    bool close();
    void set_column_type(comp_t comp_type, col_t col_type);
    bool append(const void *v);
    bool append_batch(const void *v);
    bool flush_batched();
    bool lookup_bypos(uint64_t pos, void *v);
    bool lookup_bypos(uint64_t pos, rle_triple_tuple_t *v);
    bool lookup_bypos(uint64_t pos_start, uint64_t length, std::deque<nocomp_tuple_t *> &vs);
    bool lookup_bypos(uint64_t pos_start, uint64_t length, char *buf);
    bool lookup_bypos(uint64_t pos_start, uint64_t length, std::deque<rle_triple_tuple_t *> &vs);
    bool lookup_byposrange(uint64_t pos_start, uint64_t length, std::deque<rle_triple_tuple_t *> &vs);
    bool lookup_byposrange(uint64_t pos_start, uint64_t length, std::deque<nocomp_tuple_t *> &vs);
    bool lookup_bypos(uint64_t pos, nocomp_tuple_t *v); 
    bool lookup_byposlist(std::deque<off_t> &poslist, std::deque<nocomp_tuple_t *> &vs);
    bool lookup_byposlist(std::deque<off_t> &poslist, std::deque<rle_triple_tuple_t *> &vs);
    bool get_next(void *pred, void *v);
    bool get_next(void *pred, void *v, off_t *pos);
    bool get_next(void *pred, rle_triple_tuple_t *v, off_t *pos);
    bool get_next(rle_triple_tuple_t *v, off_t *pos, void *pred, compare_t compare_type);
    bool get_next(nocomp_tuple_t *v, off_t *pos, void *pred, compare_t compare_type);
    bool get_next_block(std::deque<nocomp_tuple_t *> &vs, std::deque<off_t> &poss, void *pred, compare_t compare_type, bool pos_only);
    bool get_next_block(std::deque<rle_triple_tuple_t *> &vs, std::deque<off_t> &poss, void *pred, compare_t compare_type, bool pos_only);
    comp_t get_comp_type();
    col_t get_col_type();
    off_t get_offset();
    off_t get_read_offset();
    size_t get_val_size();
    size_t get_tuple_size();
    off_t get_n_tuples();

  private:
    bool opened_;
    int fd_;
    col_flags_t oflags_;
    column_header_t header_;
    off_t read_offset_;
    off_t read_pos_;
    Dbt pos_k_;
    Dbt pos_v_;
    Db *db_;
    char *rbuf;
    size_t buf_size_;
    bool indexed_;
    bool index_opened_;
    Db *idx_;
    Dbt idx_k_;
    Dbt idx_v_;
    char *varray_;
    off_t varray_off_;
    off_t varray_max_;
    pthread_mutex_t mutex_;
  };

  int compare_double(Db *dbp, const Dbt *a, const Dbt *b);
  int compare_uint64(Db *dbp, const Dbt *a, const Dbt *b);
  int compare_int64(Db *dbp, const Dbt *a, const Dbt *b);
  int compare_uint32(Db *dbp, const Dbt *a, const Dbt *b);
  int compare_int32(Db *dbp, const Dbt *a, const Dbt *b);
  int compare_uint8(Db *dbp, const Dbt *a, const Dbt *b);
  int compare_int8(Db *dbp, const Dbt *a, const Dbt *b);
  int generic_compare(void *v1, void *v2, col_t col_type);
  size_t get_col_size(col_t col_type);
}

#endif
