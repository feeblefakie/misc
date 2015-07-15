#ifndef OPS_H
#define OPS_H

#include <deque>
#include "column.h"

namespace cstore {

  class DS1 {
  public:
    DS1(Column *c, void *pred, compare_t compare_type);
    ~DS1();
    bool nocomp_op(off_t *pos);
    bool rle_op(off_t *pos);
    bool rle_op(off_t *pos_start, off_t *length);

  private:
    Column *c_;
    void *pred_;
    compare_t compare_type_;
    nocomp_tuple_t nocomp_tuple_;
    rle_triple_tuple_t rle_tuple_;
    off_t rle_pos_;
    off_t rle_endpos_;
    std::deque<nocomp_tuple_t *> vs_;
    std::deque<off_t> poss_;
  };

  class DS1a {
  public:
    DS1a(Column *c, void *pred, compare_t compare_type);
    ~DS1a();
    bool nocomp_op(void *val, bool *passed);
    bool rle_op(rle_triple_tuple_t *v, bool *passed);

  private:
    Column *c_;
    void *pred_;
    compare_t compare_type_;
  };

  class DS2 {
  public:
    DS2(Column *c, void *pred, compare_t compare_type, size_t buffer_size = 65536);
    ~DS2();
    bool nocomp_op(off_t *pos, void **val);
    bool rle_op(off_t *pos, void **val);
    bool rle_op(off_t *start, off_t *length, void **val);
    bool rle_op_block(std::deque<off_t> &poss, std::deque<void *> &vals);
    comp_t get_comp_type();
    void set_offset(off_t pos);

  private:
    Column *c_;
    void *pred_;
    compare_t compare_type_;
    nocomp_tuple_t nocomp_tuple_;
    rle_triple_tuple_t rle_tuple_;
    off_t rle_pos_;
    off_t rle_endpos_;
    std::deque<nocomp_tuple_t *> vs_;
    std::deque<off_t> poss_;
    off_t pos_;
    off_t block_read_pos_;
    off_t block_read_length_;
    char *buf_;
    size_t buffer_size_;
  };

  class DS3 {
  public:
    DS3(Column *c);
    ~DS3();
    bool nocomp_op(off_t pos, void **val);
    bool rle_op(off_t pos, void **val);
    bool nocomp_op(off_t start, off_t length, std::deque<nocomp_tuple_t *> &vs);
    bool nocomp_op(std::deque<off_t> &poslist, std::deque<nocomp_tuple_t *> &vs);
    bool rle_op(off_t start, off_t length, std::deque<rle_triple_tuple_t *> &vs);
    bool rle_op(std::deque<off_t> &poslist, std::deque<rle_triple_tuple_t *> &vs);

  private:
    Column *c_;
    nocomp_tuple_t nocomp_tuple_;
    rle_triple_tuple_t rle_tuple_;
  };

  class DS4 {
  public:
    DS4(Column *c, void *pred, compare_t compare_type, size_t buffer_size = 4096);
    ~DS4();
    bool nocomp_op(off_t pos, void **val, bool *passed);
    bool rle_op(off_t pos, void **val, bool *passed);
    bool nocomp_op(off_t pos, nocomp_tuple_t *v, bool *passed);
    bool rle_op(off_t pos, rle_triple_tuple_t *v, bool *passed);
    bool nocomp_op(off_t start, off_t length, std::deque<nocomp_tuple_t *> &vs);
    bool rle_op(off_t start, off_t length, std::deque<rle_triple_tuple_t *> &vs);
    bool nocomp_op_new(off_t pos, void **val, bool *passed);
    bool nocomp_op_block(std::deque<off_t> &poss, std::deque<void *> &vals);
    bool rle_op_new(off_t pos, void **val, bool *passed);
    comp_t get_comp_type();
    col_t get_col_type();

  private:
    Column *c_;
    void *pred_;
    compare_t compare_type_;
    nocomp_tuple_t nocomp_tuple_;
    rle_triple_tuple_t rle_tuple_;
    off_t block_read_pos_;
    off_t block_read_length_;
    off_t current_triple_id_;
    std::deque<nocomp_tuple_t *> vs_;
    std::deque<rle_triple_tuple_t *> vsr_;
    char *buf_;
    size_t buffer_size_;
  };

  class AND2 {
  public:
    AND2();
    ~AND2();
    void op(std::deque<off_t> pos1s, std::deque<off_t> &pos2s, std::deque<off_t> &anded);
  };

  class MERGE2 {
  public:
    MERGE2();
    ~MERGE2();
    void op(void *val1, void *val2);
  };

  class SPC2 {
  public:
    SPC2(Column *c1, void *pred1, compare_t compare_type1,
         Column *c2, void *pred2, compare_t compare_type2);
    ~SPC2();
    bool op(void **val1, void **val2, bool *passed);

  private:
    Column *c1_;
    Column *c2_;
    void *pred1_;
    void *pred2_;
    compare_t compare_type1_;
    compare_t compare_type2_;
    nocomp_tuple_t *nocomp_tuple1_;
    nocomp_tuple_t *nocomp_tuple2_;
    rle_triple_tuple_t *rle_tuple1_;
    rle_triple_tuple_t *rle_tuple2_;
    off_t rle_pos1_;
    off_t rle_pos2_;
    off_t rle_endpos1_;
    off_t rle_endpos2_;
    std::deque<rle_triple_tuple_t *> vs1r_;
    std::deque<off_t> poss1r_;
    std::deque<rle_triple_tuple_t *> vs1_;
    std::deque<off_t> poss1_;
    std::deque<rle_triple_tuple_t *> vs2r_;
    std::deque<off_t> poss2r_;
    std::deque<nocomp_tuple_t *> vs2_;
    std::deque<off_t> poss2_;
    bool c1_done_;
    bool c2_done_;
  };

}

#endif
