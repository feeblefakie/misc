#include "ops.h"

namespace cstore {

  /*
   *  DS1
   */
  DS1::DS1(Column *c, void *pred, compare_t compare_type)
  : c_(c), pred_(pred), compare_type_(compare_type), rle_pos_(0), rle_endpos_(0)
  { }

  DS1::~DS1() { }

  /*
  bool DS1::nocomp_op(off_t *pos) {
    bool ret = c_->get_next(&nocomp_tuple_, pos, pred_, compare_type_);
    return ret;
  }
  */

  bool DS1::nocomp_op(off_t *pos) {
    if (poss_.empty()) {
      if (c_->get_read_offset() >= c_->get_offset()) {
        return false;
      }
      bool ret = c_->get_next_block(vs_, poss_, pred_, compare_type_, true);
    }
    *pos = poss_.front();
    poss_.pop_front();
    return true;
  }

  bool DS1::rle_op(off_t *pos) {
    if (rle_pos_ >= rle_endpos_) {
      bool ret = c_->get_next(&rle_tuple_, pos, pred_, compare_type_);
      if (!ret) {
        return false;
      }
      rle_pos_ = rle_tuple_.start;
      rle_endpos_ = rle_tuple_.start + rle_tuple_.length;
    }
    *pos = rle_pos_++;
    return true;
  }

  bool DS1::rle_op(off_t *pos_start, off_t *length) {
    bool ret = c_->get_next(&rle_tuple_, pos_start, pred_, compare_type_);
    if (!ret) {
      return false;
    }
    *pos_start = rle_tuple_.start;
    *length = rle_tuple_.length;
    return true;
  }

  /*
   *  DS1 alpha (used in the middle of LM-pipelined)
   */
  DS1a::DS1a(Column *c, void *pred, compare_t compare_type)
  : c_(c), pred_(pred), compare_type_(compare_type)
  { }

  DS1a::~DS1a() { }

  bool DS1a::nocomp_op(void *val, bool *passed) {
    if (pred_ == NULL) {
      *passed = true;
    } else {
      switch (compare_type_) {
        case EQ:
          *passed = (generic_compare(val, pred_, c_->get_col_type()) == 0) ? true : false;
          break;
        case NE:
          *passed = (generic_compare(val, pred_, c_->get_col_type()) != 0) ? true : false;
          break;
        case GT:
          *passed = (generic_compare(val, pred_, c_->get_col_type()) > 0) ? true : false;
          break;
        case GE:
          *passed = (generic_compare(val, pred_, c_->get_col_type()) >= 0) ? true : false;
          break;
        case LT:
          *passed = (generic_compare(val, pred_, c_->get_col_type()) < 0) ? true : false;
          break;
        case LE:
          *passed = (generic_compare(val, pred_, c_->get_col_type()) <= 0) ? true : false;
          break;
        default:
          *passed = false;
      }
    }
    return true;
  }

  bool DS1a::rle_op(rle_triple_tuple_t *v, bool *passed) {
    if (pred_ == NULL) {
      *passed = true;
    } else {
      switch (compare_type_) {
        case EQ:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) == 0) ? true : false;
          break;
        case NE:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) != 0) ? true : false;
          break;
        case GT:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) > 0) ? true : false;
          break;
        case GE:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) >= 0) ? true : false;
          break;
        case LT:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) < 0) ? true : false;
          break;
        case LE:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) <= 0) ? true : false;
          break;
        default:
          *passed = false;
      }
    }
    return true;
  }

  /*
   *  DS2
   */
  DS2::DS2(Column *c, void *pred, compare_t compare_type, size_t buffer_size)
  : c_(c), pred_(pred), compare_type_(compare_type), rle_pos_(0), rle_endpos_(0), 
    pos_(0), block_read_pos_(0), block_read_length_(0), buffer_size_(buffer_size)
  {
    buf_ = new char[buffer_size];
  }

  DS2::~DS2() { }

  /*
  bool DS2::nocomp_op(off_t *pos, void **val) {
    bool ret = c_->get_next(&nocomp_tuple_, pos, pred_, compare_type_);
    *val = nocomp_tuple_.val;
    return ret;
  }
  */
  /*
  bool DS2::nocomp_op(off_t *pos, void **val) {
    while (poss_.empty()) {
      if (c_->get_read_offset() >= c_->get_offset()) {
        return false;
      }
      bool ret = c_->get_next_block(vs_, poss_, pred_, compare_type_, false);
    }
    *pos = poss_.front();
    poss_.pop_front();
    *val = vs_.front();
    vs_.pop_front();
    return true;
  }
  */
  bool DS2::nocomp_op(off_t *pos, void **val) {
    while (pos_ < c_->get_n_tuples()) {
      if (pos_ >= block_read_pos_ && pos_ < block_read_pos_ + block_read_length_) {
        // already read
      } else {
        off_t length = buffer_size_ / sizeof(nocomp_tuple_);
        //off_t length = 1048576 / sizeof(nocomp_tuple_);
        bool ret = c_->lookup_bypos(pos_, length, buf_);
        if (!ret) {
          return false;
        }
        block_read_pos_ = pos_;
        block_read_length_ = length;
      }
      off_t tuple_size = c_->get_tuple_size();
      memcpy(&nocomp_tuple_, (const void *) (buf_ + tuple_size * (pos_ - block_read_pos_)), tuple_size);
      *val = &nocomp_tuple_;
      *pos = pos_;
      pos_++;

      bool passed = true;
      if (pred_ != NULL) {
        switch (compare_type_) {
          case EQ:
            passed = (generic_compare(*val, pred_, c_->get_col_type()) == 0) ? true : false;
            break;
          case NE:
            passed = (generic_compare(*val, pred_, c_->get_col_type()) != 0) ? true : false;
            break;
          case GT:
            passed = (generic_compare(*val, pred_, c_->get_col_type()) > 0) ? true : false;
            break;
          case GE:
            passed = (generic_compare(*val, pred_, c_->get_col_type()) >= 0) ? true : false;
            break;
          case LT:
            passed = (generic_compare(*val, pred_, c_->get_col_type()) < 0) ? true : false;
            break;
          case LE:
            passed = (generic_compare(*val, pred_, c_->get_col_type()) <= 0) ? true : false;
            break;
          default:
            passed = false;
        }
      }
      if (passed) {
        return true;
      }
    }
    return false;
  }

  bool DS2::rle_op(off_t *pos, void **val) {
    bool ret;
    if (rle_pos_ >= rle_endpos_) {
      ret = c_->get_next(&rle_tuple_, pos, pred_, compare_type_);
      if (!ret) {
        return false;
      }
      rle_pos_ = rle_tuple_.start;
      rle_endpos_ = rle_tuple_.start + rle_tuple_.length;
    }
    *val = rle_tuple_.val;
    *pos = rle_pos_++;
    return true;
  }

  bool DS2::rle_op_block(std::deque<off_t> &poss, std::deque<void *> &vals) {
    int i = 0;
    bool ret;
    while (i++ < 1000) {
      if (rle_pos_ >= rle_endpos_) {
        off_t pos;
        ret = c_->get_next(&rle_tuple_, &pos, pred_, compare_type_);
        if (!ret) {
          return false;
        }
        rle_pos_ = rle_tuple_.start;
        rle_endpos_ = rle_tuple_.start + rle_tuple_.length;
      }
      char *val = new char[c_->get_val_size()];
      memcpy(val, rle_tuple_.val, c_->get_val_size());
      vals.push_back(val);
      poss.push_back(rle_pos_);
      rle_pos_++;
    }
    return true;
  }

  bool DS2::rle_op(off_t *start, off_t *length, void **val) {
    bool ret;
    ret = c_->get_next(&rle_tuple_, start, pred_, compare_type_);
    if (!ret) {
      return false;
    }
    *val = rle_tuple_.val;
    *start = rle_tuple_.start;
    *length = rle_tuple_.length;
    return true;
  }

  comp_t DS2::get_comp_type() {
    return c_->get_comp_type();
  }

  void DS2::set_offset(off_t pos) {
    pos_ = pos;
  }

  /*
   *  DS3
   */
  DS3::DS3(Column *c)
  : c_(c)
  { }

  DS3::~DS3() { }

  bool DS3::nocomp_op(off_t pos, void **val) {
    bool ret = c_->lookup_bypos(pos, &nocomp_tuple_);
    *val = nocomp_tuple_.val;
    return ret;
  }

  bool DS3::rle_op(off_t pos, void **val) {
    bool ret = c_->lookup_bypos(pos, &rle_tuple_);
    if (!ret) {
      return false;
    }
    *val = rle_tuple_.val;
    return true;
  }

  bool DS3::nocomp_op(off_t start, off_t length, std::deque<nocomp_tuple_t *> &vs) {
    bool ret = c_->lookup_byposrange(start, length, vs);
    if (!ret) {
      return false;
    }
    return true;
  }

  bool DS3::nocomp_op(std::deque<off_t> &poslist, std::deque<nocomp_tuple_t *> &vs) {
    bool ret = c_->lookup_byposlist(poslist, vs);
    for (int i = 0; i < vs.size(); ++i) {
      nocomp_tuple_t *t = vs.at(i);
    }
    if (!ret) {
      return false;
    }
    return true;
  }

  bool DS3::rle_op(off_t start, off_t length, std::deque<rle_triple_tuple_t *> &vs) {
    bool ret = c_->lookup_bypos(start, length, vs);
    if (!ret) {
      return false;
    }
    return true;
  }

  bool DS3::rle_op(std::deque<off_t> &poslist, std::deque<rle_triple_tuple_t *> &vs) {
    bool ret = c_->lookup_byposlist(poslist, vs);
    if (!ret) {
      return false;
    }
    return true;
  }

  /*
   *  DS4
   */
  DS4::DS4(Column *c, void *pred, compare_t compare_type, size_t buffer_size)
  : c_(c), pred_(pred), compare_type_(compare_type), block_read_pos_(0), block_read_length_(0), buffer_size_(buffer_size)
  { 
    buf_ = new char[buffer_size];
    memset(buf_, 0, buffer_size);
  }

  DS4::~DS4() { }

  bool DS4::nocomp_op(off_t pos, void **val, bool *passed) {
    bool ret = c_->lookup_bypos(pos, &nocomp_tuple_);
    if (!ret) {
      return false;
    }
    *val = nocomp_tuple_.val;
    if (pred_ == NULL) {
      *passed = true;
    } else {
      switch (compare_type_) {
        case EQ:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) == 0) ? true : false;
          break;
        case NE:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) != 0) ? true : false;
          break;
        case GT:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) > 0) ? true : false;
          break;
        case GE:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) >= 0) ? true : false;
          break;
        case LT:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) < 0) ? true : false;
          break;
        case LE:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) <= 0) ? true : false;
          break;
        default:
          *passed = false;
      }
    }
    return true;
  }

  bool DS4::nocomp_op(off_t pos, nocomp_tuple_t *v, bool *passed) {
    bool ret = c_->lookup_bypos(pos, v);
    if (!ret) {
      return false;
    }
    if (pred_ == NULL) {
      *passed = true;
    } else {
      switch (compare_type_) {
        case EQ:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) == 0) ? true : false;
          break;
        case NE:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) != 0) ? true : false;
          break;
        case GT:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) > 0) ? true : false;
          break;
        case GE:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) >= 0) ? true : false;
          break;
        case LT:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) < 0) ? true : false;
          break;
        case LE:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) <= 0) ? true : false;
          break;
        default:
          *passed = false;
      }
    }
    return true;
  }

  bool DS4::nocomp_op_new(off_t pos, void **val, bool *passed) {
    if (pos >= block_read_pos_ && pos < block_read_pos_ + block_read_length_) {
      // already read
    } else {
      /*
      for (std::deque<nocomp_tuple_t *>::iterator itr = vs_.begin();
           itr != vs_.end(); ++itr) {
        delete(*itr);
      }
      vs_.clear();
      */
      //off_t length = 1048576 / sizeof(nocomp_tuple_);
      //off_t length = 65536 / sizeof(nocomp_tuple_);
      off_t length = buffer_size_ / sizeof(nocomp_tuple_);
      //bool ret = c_->lookup_bypos(pos, length, vs_);
      bool ret = c_->lookup_bypos(pos, length, buf_);
      /*
      if (vs_.empty()) {
        return false;
      }
      */
      if (!ret) { 
        return false;
      }
      block_read_pos_ = pos;
      block_read_length_ = length;
    }
    //*val = vs_.at(pos - block_read_pos_);
    off_t tuple_size = c_->get_tuple_size();
    memcpy(&nocomp_tuple_, (const void *) (buf_ + tuple_size * (pos - block_read_pos_)), tuple_size);
    *val = &nocomp_tuple_;
    //memcpy(*val, (const void *) (buf_ + tuple_size * (pos - block_read_pos_)), tuple_size);

    /*
    bool ret = c_->lookup_bypos(pos, &nocomp_tuple_);
    if (!ret) {
      return false;
    }
    *val = nocomp_tuple_.val;
    */
    if (pred_ == NULL) {
      *passed = true;
    } else {
      switch (compare_type_) {
        case EQ:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) == 0) ? true : false;
          break;
        case NE:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) != 0) ? true : false;
          break;
        case GT:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) > 0) ? true : false;
          break;
        case GE:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) >= 0) ? true : false;
          break;
        case LT:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) < 0) ? true : false;
          break;
        case LE:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) <= 0) ? true : false;
          break;
        default:
          *passed = false;
      }
    }
    return true;
  }

  bool DS4::rle_op_new(off_t pos, void **val, bool *passed) {
    if (pos >= block_read_pos_ && pos < block_read_pos_ + block_read_length_) {
      // already read
    } else {
      for (std::deque<rle_triple_tuple_t *>::iterator itr = vsr_.begin();
           itr != vsr_.end(); ++itr) {
        delete(*itr);
      }
      vsr_.clear();
      off_t length = 1048576 / sizeof(rle_triple_tuple_t);
      //off_t length = 65536 / sizeof(rle_triple_tuple_t);
      bool ret = c_->lookup_bypos(pos, length, vsr_);
      if (vsr_.empty()) {
        return false;
      }
      current_triple_id_ = 0;
      block_read_pos_ = vsr_.front()->start;
      block_read_length_ = vsr_.back()->start + vsr_.back()->length - block_read_pos_;
    }

    rle_triple_tuple_t *t = vsr_.at(current_triple_id_);
    if (pos >= t->start && pos < t->start + t->length) {
      *val = t->val;
    } else {
      // fallback to linear search
      for (int i = 0; i < vsr_.size(); ++i) {
        rle_triple_tuple_t *t = vsr_.at(i);
        if (pos >= t->start && pos < t->start + t->length) {
          *val = t->val;
          current_triple_id_ = i;
          break;
        }
      }
    }

    /*
    bool ret = c_->lookup_bypos(pos, &nocomp_tuple_);
    if (!ret) {
      return false;
    }
    *val = nocomp_tuple_.val;
    */
    if (pred_ == NULL) {
      *passed = true;
    } else {
      switch (compare_type_) {
        case EQ:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) == 0) ? true : false;
          break;
        case NE:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) != 0) ? true : false;
          break;
        case GT:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) > 0) ? true : false;
          break;
        case GE:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) >= 0) ? true : false;
          break;
        case LT:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) < 0) ? true : false;
          break;
        case LE:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) <= 0) ? true : false;
          break;
        default:
          *passed = false;
      }
    }
    return true;
  }

  bool DS4::nocomp_op_block(std::deque<off_t> &poss, std::deque<void *> &vals) {
    for (int i = 0; i < poss.size(); ++i) {
      bool ret = c_->lookup_bypos(poss.at(i), &nocomp_tuple_);
      if (!ret) {
        return false;
      }
      char *v = nocomp_tuple_.val;
      if (pred_ == NULL) {
        // do nothing
      } else {
        switch (compare_type_) {
          case EQ:
            if (generic_compare(v, pred_, c_->get_col_type()) == 0) {
              break;
            } else {
              continue;
            }
          case NE:
            if (generic_compare(v, pred_, c_->get_col_type()) != 0) {
              break;
            } else {
              continue;
            }
          case GT:
            if (generic_compare(v, pred_, c_->get_col_type()) > 0) {
              break;
            } else {
              continue;
            }
          case GE:
            if (generic_compare(v, pred_, c_->get_col_type()) >= 0) {
              break;
            } else {
              continue;
            }
          case LT:
            if (generic_compare(v, pred_, c_->get_col_type()) < 0) {
              break;
            } else {
              continue;
            }
          case LE:
            if (generic_compare(v, pred_, c_->get_col_type()) <= 0) {
              break;
            } else {
              continue;
            }
        }
      }
      char *val = new char[c_->get_val_size()];
      memcpy(val, nocomp_tuple_.val, c_->get_val_size());
    }
    return true;
  }

  bool DS4::rle_op(off_t pos, void **val, bool *passed) {
    bool ret = c_->lookup_bypos(pos, &rle_tuple_);
    if (!ret) {
      return false;
    }
    *val = rle_tuple_.val;
    if (pred_ == NULL) {
      *passed = true;
    } else {
      switch (compare_type_) {
        case EQ:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) == 0) ? true : false;
          break;
        case NE:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) != 0) ? true : false;
          break;
        case GT:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) > 0) ? true : false;
          break;
        case GE:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) >= 0) ? true : false;
          break;
        case LT:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) < 0) ? true : false;
          break;
        case LE:
          *passed = (generic_compare(*val, pred_, c_->get_col_type()) <= 0) ? true : false;
          break;
        default:
          *passed = false;
      }
    }
    return true;
  }

  bool DS4::rle_op(off_t pos, rle_triple_tuple_t *v, bool *passed) {
    bool ret = c_->lookup_bypos(pos, v);
    if (!ret) {
      return false;
    }
    if (pred_ == NULL) {
      *passed = true;
    } else {
      switch (compare_type_) {
        case EQ:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) == 0) ? true : false;
          break;
        case NE:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) != 0) ? true : false;
          break;
        case GT:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) > 0) ? true : false;
          break;
        case GE:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) >= 0) ? true : false;
          break;
        case LT:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) < 0) ? true : false;
          break;
        case LE:
          *passed = (generic_compare(v->val, pred_, c_->get_col_type()) <= 0) ? true : false;
          break;
        default:
          *passed = false;
      }
    }
    return true;
  }

  bool DS4::nocomp_op(off_t start, off_t length, std::deque<nocomp_tuple_t *> &vs) {
    bool ret = c_->lookup_byposrange(start, length, vs);
    if (!ret) {
      return false;
    }
    if (pred_ == NULL) {
      // do nothing
    } else {
      std::deque<nocomp_tuple_t *>::iterator itr = vs.begin();
      for (; itr != vs.end(); ++itr) {
        char *val = (*itr)->val;
        bool passed;
        switch (compare_type_) {
          case EQ:
            passed = (generic_compare(val, pred_, c_->get_col_type()) == 0) ? true : false;
            break;
          case NE:
            passed = (generic_compare(val, pred_, c_->get_col_type()) != 0) ? true : false;
            break;
          case GT:
            passed = (generic_compare(val, pred_, c_->get_col_type()) > 0) ? true : false;
            break;
          case GE:
            passed = (generic_compare(val, pred_, c_->get_col_type()) >= 0) ? true : false;
            break;
          case LT:
            passed = (generic_compare(val, pred_, c_->get_col_type()) < 0) ? true : false;
            break;
          case LE:
            passed = (generic_compare(val, pred_, c_->get_col_type()) <= 0) ? true : false;
            break;
          default:
            passed = false;
        }
        if (!passed) {
          vs.erase(itr);
        }
      }
    }
    return true;
  }

  bool DS4::rle_op(off_t start, off_t length, std::deque<rle_triple_tuple_t *> &vs) {
    bool ret = c_->lookup_byposrange(start, length, vs);
    if (!ret) {
      return false;
    }
    if (pred_ == NULL) {
      // do nothing
    } else {
      std::deque<rle_triple_tuple_t *>::iterator itr = vs.begin();
      for (; itr != vs.end(); ++itr) {
        char *val = (*itr)->val;
        bool passed;
        switch (compare_type_) {
          case EQ:
            passed = (generic_compare(val, pred_, c_->get_col_type()) == 0) ? true : false;
            break;
          case NE:
            passed = (generic_compare(val, pred_, c_->get_col_type()) != 0) ? true : false;
            break;
          case GT:
            passed = (generic_compare(val, pred_, c_->get_col_type()) > 0) ? true : false;
            break;
          case GE:
            passed = (generic_compare(val, pred_, c_->get_col_type()) >= 0) ? true : false;
            break;
          case LT:
            passed = (generic_compare(val, pred_, c_->get_col_type()) < 0) ? true : false;
            break;
          case LE:
            passed = (generic_compare(val, pred_, c_->get_col_type()) <= 0) ? true : false;
            break;
          default:
            passed = false;
        }
        if (!passed) {
          vs.erase(itr);
        }
      }
    }
    return true;
  }

  col_t DS4::get_col_type() {
    return c_->get_col_type();
  }

  comp_t DS4::get_comp_type() {
    return c_->get_comp_type();
  }

  /*
   * AND2
   */
  AND2::AND2()
  { }

  AND2::~AND2() { }

  void AND2::op(std::deque<off_t> pos1s, std::deque<off_t> &pos2s, std::deque<off_t> &anded) {
    off_t pos1 = pos1s.front();
    pos1s.pop_front();
    off_t pos2 = pos2s.front();
    pos2s.pop_front();
    while (!pos1s.empty() && !pos2s.empty()) {
      if (pos1 < pos2) {
        pos1 = pos1s.front();
        pos1s.pop_front();
      } else if (pos2 < pos1) {
        pos2 = pos2s.front();
        pos2s.pop_front();
      } else {
        anded.push_back(pos1);
        pos1 = pos1s.front();
        pos1s.pop_front();
        pos2 = pos2s.front();
        pos2s.pop_front();
      }
    }
  }

  /*
   * MERGE2
   */
  MERGE2::MERGE2()
  { }

  MERGE2::~MERGE2() { }

  void MERGE2::op(void *val1, void *val2) {
    // do nothing for now
  }

  /*
   * SPC
   */
  SPC2::SPC2(Column *c1, void *pred1, compare_t compare_type1,
             Column *c2, void *pred2, compare_t compare_type2)
  : c1_(c1), pred1_(pred1), compare_type1_(compare_type1), rle_pos1_(0), rle_endpos1_(0),
    c2_(c2), pred2_(pred2), compare_type2_(compare_type2), rle_pos2_(0), rle_endpos2_(0),
    c1_done_(false), c2_done_(false)
  { }

  SPC2::~SPC2() { }

  bool SPC2::op(void **val1, void **val2, bool *passed) {
    bool ret1 = true;
    bool ret2 = true;
    off_t pos1, pos2;
    if (c1_->get_comp_type() == RLE) {
      if (rle_pos1_ >= rle_endpos1_) {
        if (vs1r_.empty()) {
          if (c1_done_) {
            return false;
          }
          ret1 = c1_->get_next_block(vs1r_, poss1r_, NULL, compare_type1_, false);
          if (!ret1) { c1_done_ = true; }
        }
        rle_tuple1_ = vs1r_.front();
        vs1r_.pop_front();
        rle_pos1_ = rle_tuple1_->start;
        rle_endpos1_ = rle_tuple1_->start + rle_tuple1_->length;
        /*
        ret1 = c1_->get_next(&rle_tuple1_, &pos1, NULL, compare_type1_);
        rle_pos1_ = rle_tuple1_.start;
        rle_endpos1_ = rle_tuple1_.start + rle_tuple1_.length;
        */
      }
      /*
      *val1 = rle_tuple1_.val;
      pos1 = rle_pos1_++;
      */
      *val1 = rle_tuple1_->val;
      pos1 = rle_pos1_++;
    } else {
      /*
      ret1 = c1_->get_next(&nocomp_tuple1_, &pos1, NULL, compare_type1_);
      *val1 = nocomp_tuple1_.val;
      */
      if (vs1_.empty()) {
        ret2 = c1_->get_next_block(vs1_, poss1_, NULL, compare_type1_, false);
      }
      *val1 = vs1_.front();
      vs1_.pop_front();
      pos1 = poss1_.front();
      poss1_.pop_front();
    }
    if (!ret1) {
      return false;
    }

    if (c2_->get_comp_type() == RLE) {
      if (rle_pos2_ >= rle_endpos2_) {
        if (vs2r_.empty()) {
          if (c2_done_) {
            return false;
          }
          ret2 = c2_->get_next_block(vs2r_, poss2r_, NULL, compare_type2_, false);
          if (!ret2) { c2_done_ = true; }
        }
        rle_tuple2_ = vs2r_.front();
        vs2r_.pop_front();
        rle_pos2_ = rle_tuple2_->start;
        rle_endpos2_ = rle_tuple2_->start + rle_tuple2_->length;
      }
      *val2 = rle_tuple2_->val;
      pos2 = rle_pos2_++;
    } else {
      if (vs2_.empty()) {
        ret2 = c2_->get_next_block(vs2_, poss2_, NULL, compare_type2_, false);
      }
      *val2 = vs2_.front();
      vs2_.pop_front();
      pos2 = poss2_.front();
      poss2_.pop_front();
    }
    if (pos1 != pos2) {
      std::cerr << "something wrong in get_next: pos1=" << pos1 << ", pos2=" << pos2 << std::endl;
      return false;
    }
    if (!ret2) {
      return false;
    }

    bool val1_passed, val2_passed;
    // apply predicate
    if (pred1_ == NULL) {
      val1_passed = true;
    } else {
      switch (compare_type1_) {
        case EQ:
          val1_passed = (generic_compare(*val1, pred1_, c1_->get_col_type()) == 0) ? true : false;
          break;
        case NE:
          val1_passed = (generic_compare(*val1, pred1_, c1_->get_col_type()) != 0) ? true : false; 
          break;
        case GT:
          val1_passed = (generic_compare(*val1, pred1_, c1_->get_col_type()) > 0) ? true : false;
          break;
        case GE:
          val1_passed = (generic_compare(*val1, pred1_, c1_->get_col_type()) >= 0) ? true : false;
          break;
        case LT:
          val1_passed = (generic_compare(*val1, pred1_, c1_->get_col_type()) < 0) ? true : false;
          break;
        case LE:
          val1_passed = (generic_compare(*val1, pred1_, c1_->get_col_type()) <= 0) ? true : false;
          break;
      }
    }
    if (pred2_ == NULL) {
      val2_passed = true;
    } else {
      switch (compare_type2_) {
        case EQ:
          val2_passed = (generic_compare(*val2, pred2_, c2_->get_col_type()) == 0) ? true : false;
          break;
        case NE:
          val2_passed = (generic_compare(*val2, pred2_, c2_->get_col_type()) != 0) ? true : false; 
          break;
        case GT:
          val2_passed = (generic_compare(*val2, pred2_, c2_->get_col_type()) > 0) ? true : false;
          break;
        case GE:
          val2_passed = (generic_compare(*val2, pred2_, c2_->get_col_type()) >= 0) ? true : false;
          break;
        case LT:
          val2_passed = (generic_compare(*val2, pred2_, c2_->get_col_type()) < 0) ? true : false;
          break;
        case LE:
          val2_passed = (generic_compare(*val2, pred2_, c2_->get_col_type()) <= 0) ? true : false;
          break;
      }
    }

    if (val1_passed && val2_passed) {
      *passed = true;
    } else {
      *passed = false;
    }
    return true;
  }

}

