#include "column.h"

namespace cstore {

  Column::Column()
  : varray_(NULL), index_opened_(false)
  { 
    memset(&header_, 0, sizeof(column_header_t));
    //rbuf = new char[10*1024*1024];
    buf_size_ = DEFAULT_BUFSIZE;
    rbuf = new char[buf_size_];
    //pthread_mutex_init(&mutex_, NULL);
  }

  Column::~Column()
  {
    if (opened_) {
      close();
    }
  }

  bool Column::open(std::string col_name, col_flags_t oflags, bool indexed) {
    // open

    fd_ = ut::_open(col_name.c_str(), oflags, 00644);
    if (fd_ < 0) { 
      error_log("open failed.");
      return false;
    }    
    opened_ = true;
    oflags_ = oflags;

    struct stat stat_buf;
    if (fstat(fd_, &stat_buf) == -1 || !S_ISREG(stat_buf.st_mode)) {
      error_log("fstat failed.");
      return false;
    }    

    if (stat_buf.st_size == 0 && oflags & COL_CREAT) {
      // newly created file
      header_.offset = sizeof(column_header_t);
      header_.n_tuples = 0;
      header_.indexed = indexed;
    } else {
      // already created file
      if (ut::_read(fd_, &header_, sizeof(column_header_t)) < 0) {
        error_log("read failed.");
        return false;
      }
    }
    read_offset_ = sizeof(column_header_t);
    read_pos_ = 0;

    if (header_.comp_type == RLE) {
      db_ = new Db(NULL, (u_int32_t) 0); 
      if (oflags & COL_CREAT) {
        db_->set_cachesize(1, 512*1024*1024, 1); 
      } else {
        db_->set_cachesize(0, 65536, 1); 
      }
      db_->set_bt_compare(compare_uint64);
      u_int32_t oFlags = DB_RDONLY;
      if (oflags & COL_CREAT) {
        oFlags = DB_CREATE; // Open flags;
      } else if (oflags & COL_RDWR) {
        oFlags = DB_CREATE; // Open flags;
      }
      std::string pos_idx_name = col_name + ".posidx";
      db_->open(NULL, pos_idx_name.c_str(), NULL, DB_BTREE, oFlags, 0);

      memset(&pos_k_, 0, sizeof(Dbt)); 
      memset(&pos_v_, 0, sizeof(Dbt)); 
      pos_k_.set_size(sizeof(off_t));
      pos_v_.set_size(sizeof(off_t));
    }

    // TODO: need to specify if the column is index or not every time for now
    //if (header_.indexed) {
    if (indexed) {
      idx_ = new Db(NULL, (u_int32_t) 0); 
      //idx_->set_cachesize(4, 0, 1); 
      switch (header_.col_type) {
        case TINYINT:
          idx_->set_bt_compare(compare_int8);
          break;
        case INT:
          idx_->set_bt_compare(compare_int32);
          break;
        case BIGINT:
          idx_->set_bt_compare(compare_int64);
          break;
        case DOUBLE:
          idx_->set_bt_compare(compare_double);
          break;
      }
      u_int32_t oFlags = DB_RDONLY;
      if (oflags & COL_CREAT) {
        oFlags = DB_CREATE; // Open flags;
      } else if (oflags & COL_RDWR) {
        oFlags = DB_CREATE; // Open flags;
      }
      std::string idx_name = col_name + ".idx";
      idx_->set_flags(DB_DUP);
      idx_->open(NULL, idx_name.c_str(), NULL, DB_BTREE, oFlags, 0);
      index_opened_ = true;
    }

    return true;
  }

  bool Column::close() {
    if (oflags_ & COL_CREAT || oflags_ & COL_RDWR) {
      if (header_.comp_type != NOCOMP) {
        db_->sync(0);
      }
      if (index_opened_) {
        idx_->sync(0);
      }
      ut::_pwrite(fd_, &header_, sizeof(column_header_t), 0);
      fsync(fd_);
    }
    if (header_.comp_type != NOCOMP) {
      db_->close(0);
    }
    if (index_opened_) {
      idx_->close(0);
    }
    ::close(fd_);
    opened_ = false;
  }
  
  void Column::set_column_type(comp_t comp_type, col_t col_type) {
    header_.comp_type = comp_type;
    header_.col_type = col_type;
    if (comp_type == NOCOMP) {
      switch (col_type) {
        case TINYINT:
        case CHAR:
          header_.val_size = 1;
          header_.tuple_size = sizeof(nocomp_val1_t);
          break;
        case INT:
        case CHAR4:
          header_.val_size = 4;
          header_.tuple_size = sizeof(nocomp_val4_t);
          break;
        case BIGINT:
        case CHAR8:
        case DOUBLE:
          header_.val_size = 8;
          header_.tuple_size = sizeof(nocomp_val8_t);
          break;
      }
    } else if (header_.comp_type == RLE) {
      switch (col_type) {
        case TINYINT:
        case CHAR:
          header_.val_size = 1;
          header_.tuple_size = sizeof(rle_triple_val1_t);
          break;
        case INT:
        case CHAR4:
          header_.val_size = 4;
          header_.tuple_size = sizeof(rle_triple_val4_t);
          break;
        case BIGINT:
        case CHAR8:
        case DOUBLE:
          header_.val_size = 8;
          header_.tuple_size = sizeof(rle_triple_val8_t);
          break;
      }
    }
  }

  bool Column::append(const void *v) {

    bool ret = ut::_pwrite(fd_, v, header_.tuple_size, header_.offset);
    if (!ret) {
      return ret;
    }

    if (header_.comp_type == RLE) {
      switch (header_.col_type) {
        case TINYINT:
        case CHAR:
          header_.n_tuples += ((rle_triple_val1_t *) v)->length ;
          break;
        case INT:
        case CHAR4:
          header_.n_tuples += ((rle_triple_val4_t *) v)->length ;
          break;
        case BIGINT:
        case CHAR8:
        case DOUBLE:
          header_.n_tuples += ((rle_triple_val8_t *) v)->length ;
          break;
      }

      // NOTICE: n_tuples addition must be placed before updating position index,
      // because DB_SET_RANGE option finds key/data pair which is the smallest key greater than or equal to the specified key.
      header_.n_tuples--;
      pos_k_.set_data(&header_.n_tuples);
      pos_v_.set_data(&header_.offset);
      int status = db_->put(NULL, &pos_k_, &pos_v_, DB_NOOVERWRITE);
      if (status == DB_KEYEXIST) {
        db_->err(status, "Put failed because key %ld already exists", header_.offset);
      }
    }

    if (header_.indexed) {
      memset(&idx_k_, 0, sizeof(Dbt)); 
      memset(&idx_v_, 0, sizeof(Dbt)); 
      idx_k_.set_size(header_.val_size);
      idx_v_.set_size(sizeof(off_t));
      idx_k_.set_data((void *) v);
      idx_v_.set_data(&header_.n_tuples);
      int status = idx_->put(NULL, &idx_k_, &idx_v_, 0);
      if (status != 0) {
        idx_->err(status, "Put failed for some reason.");
      }
      //std::cout << *(int64_t *) v << " " << header_.n_tuples << std::endl;
    }

    header_.n_tuples++;
    header_.offset += header_.tuple_size;

    return true;
  }

  bool Column::append_batch(const void *v) {
    if (varray_ == NULL) {
      varray_max_ = 1024*1024*64; // 64MB
      varray_ = new char[varray_max_];
      varray_off_ = 0;
    }

    memcpy(varray_ + varray_off_, v, header_.tuple_size);
    varray_off_ += header_.tuple_size;
    
    if (varray_off_ + header_.tuple_size > varray_max_) {
      flush_batched();
    }

    if (header_.comp_type == RLE) {
      switch (header_.col_type) {
        case TINYINT:
        case CHAR:
          header_.n_tuples += ((rle_triple_val1_t *) v)->length ;
          break;
        case INT:
        case CHAR4:
          header_.n_tuples += ((rle_triple_val4_t *) v)->length ;
          break;
        case BIGINT:
        case CHAR8:
        case DOUBLE:
          header_.n_tuples += ((rle_triple_val8_t *) v)->length ;
          break;
      }

      // NOTICE: n_tuples addition must be placed before updating position index,
      // because DB_SET_RANGE option finds key/data pair which is the smallest key greater than or equal to the specified key.
      header_.n_tuples--;
      pos_k_.set_data(&header_.n_tuples);
      pos_v_.set_data(&header_.offset);
      int status = db_->put(NULL, &pos_k_, &pos_v_, DB_NOOVERWRITE);
      if (status == DB_KEYEXIST) {
        db_->err(status, "Put failed because key %ld already exists", header_.offset);
      }
    }

    if (header_.indexed) {
      memset(&idx_k_, 0, sizeof(Dbt)); 
      memset(&idx_v_, 0, sizeof(Dbt)); 
      idx_k_.set_size(header_.val_size);
      idx_v_.set_size(sizeof(off_t));
      idx_k_.set_data((void *) v);
      idx_v_.set_data(&header_.n_tuples);
      int status = idx_->put(NULL, &idx_k_, &idx_v_, 0);
      if (status != 0) {
        idx_->err(status, "Put failed for some reason.");
      }
    }

    header_.n_tuples++;
    header_.offset += header_.tuple_size;

    return true;
  }
  
  bool Column::flush_batched() {
    bool ret = ut::_pwrite(fd_, varray_, varray_off_, header_.offset);
    if (!ret) {
      return ret;
    }
    varray_off_ = 0;
  }

  bool Column::lookup_bypos(uint64_t pos, void *v) {
    if (header_.comp_type == NOCOMP) {
      return ut::_pread(fd_, v, header_.tuple_size, sizeof(column_header_t) + pos * header_.tuple_size);
    } else if (header_.comp_type == RLE) {
      off_t off;
      Dbt pos_k;
      Dbt pos_v;

      memset(&pos_k, 0, sizeof(Dbt)); 
      memset(&pos_v, 0, sizeof(Dbt)); 
      pos_k.set_size(sizeof(off_t));
      pos_v.set_size(sizeof(off_t));

      Dbc *cursorp;
      pos_k.set_data(&pos);
      pos_v.set_data(&off);
      pos_v.set_ulen(sizeof(off_t));
      pos_v.set_flags(DB_DBT_USERMEM);
      db_->cursor(NULL, &cursorp, 0);
      int ret = cursorp->get(&pos_k, &pos_v, DB_SET_RANGE);
      cursorp->close();
      if (ret != 0) {
        db_->err(ret, "Get failed for %d", pos);
        return false;
      } 
      return ut::_pread(fd_, v, header_.tuple_size, off);
    }
  }

  bool Column::lookup_bypos(uint64_t pos, rle_triple_tuple_t *v) {
    //std::cout << "lookup_bypos(rle) - pos: " << pos << std::endl;
    off_t off;
    Dbt pos_k;
    Dbt pos_v;

    memset(&pos_k, 0, sizeof(Dbt)); 
    memset(&pos_v, 0, sizeof(Dbt)); 
    pos_k.set_size(sizeof(off_t));
    pos_v.set_size(sizeof(off_t));

    Dbc *cursorp;
    pos_k.set_data(&pos);
    pos_v.set_data(&off);
    pos_v.set_ulen(sizeof(off_t));
    pos_v.set_flags(DB_DBT_USERMEM);
    db_->cursor(NULL, &cursorp, 0);
    int ret = cursorp->get(&pos_k, &pos_v, DB_SET_RANGE);
    cursorp->close();
    if (ret != 0) {
      db_->err(ret, "Get failed for %d", pos);
      return false;
    } 
    ut::_pread(fd_, rbuf, header_.tuple_size, off);
    memcpy(v->val, rbuf, header_.val_size);
    memcpy(&v->start, rbuf+header_.val_size, sizeof(off_t));
    memcpy(&v->length, rbuf+header_.val_size+sizeof(off_t), sizeof(off_t));
    return true;
  }

  bool Column::lookup_bypos(uint64_t pos_start, uint64_t length, std::deque<rle_triple_tuple_t *> &vs) {
    //std::cout << "lookup_bypos(rle) - pos: " << pos << std::endl;
    off_t off;
    Dbt pos_k;
    Dbt pos_v;

    memset(&pos_k, 0, sizeof(Dbt)); 
    memset(&pos_v, 0, sizeof(Dbt)); 
    pos_k.set_size(sizeof(off_t));
    pos_v.set_size(sizeof(off_t));

    Dbc *cursorp;
    pos_k.set_data(&pos_start);
    pos_v.set_data(&off);
    pos_v.set_ulen(sizeof(off_t));
    pos_v.set_flags(DB_DBT_USERMEM);
    db_->cursor(NULL, &cursorp, 0);
    int ret = cursorp->get(&pos_k, &pos_v, DB_SET_RANGE);
    cursorp->close();
    if (ret != 0) {
      db_->err(ret, "Get failed for %d", pos_start);
      return false;
    } 
  
    // TODO: Does it work in the following case ?
    // second column's RLE range is wider than the first column's RLE range.
    bool reading = true;
    while (off < header_.offset && reading) {
      off_t read_size = (header_.offset - off >= buf_size_) ? buf_size_ : header_.offset - off;
      ut::_pread(fd_, rbuf, read_size, off);
      off += read_size;
      off_t num_entry = read_size / header_.tuple_size;
      for (int i = 0; i < num_entry; ++i) {
        rle_triple_tuple_t *v = new rle_triple_tuple_t();
        memcpy(v->val, rbuf + header_.tuple_size * i, header_.val_size);
        memcpy(&v->start, rbuf + header_.tuple_size * i + header_.val_size, sizeof(off_t));
        memcpy(&v->length, rbuf + header_.tuple_size * i + header_.val_size + sizeof(off_t), sizeof(off_t));
        vs.push_back(v);
        if (pos_start + length <= v->start + v->length) {
          reading = false;
          break;
        }
      }
    }
    return true;
  }

  bool Column::lookup_byposrange(uint64_t pos_start, uint64_t length, std::deque<rle_triple_tuple_t *> &vs) {
    lookup_bypos(pos_start, length, vs);
    return true;
  }

  bool Column::lookup_byposlist(std::deque<off_t> &poslist, std::deque<rle_triple_tuple_t *> &vs) {
    off_t start = poslist.front();
    off_t end = poslist.back();
    off_t length = end - start + 1;
    std::deque<rle_triple_tuple_t *> vstmp;
    lookup_byposrange(start, length, vstmp);
    std::cout << "vssize: " << vstmp.size() << std::endl;
    std::deque<off_t>::iterator itr = poslist.begin(); 
    std::deque<rle_triple_tuple_t *>::iterator itrv = vstmp.begin(); 
    while (itr != poslist.end() && itrv != vstmp.end()) {
      //std::cout << "*itr: " << *itr << ", start: " << (*itrv)->start << ", length: " << (*itrv)->length << std::endl;
      if (*itr >= (*itrv)->start && *itr < (*itrv)->start + (*itrv)->length) {
        vs.push_back(*itrv);
        itr++;
      } else if (*itr >= (*itrv)->start + (*itrv)->length) {
        itrv++;
      } else {
        itr++;
      }
    }
    std::cout << "vstmpsize: " << vstmp.size() << std::endl;
    /*
    for (std::deque<rle_triple_tuple_t *>::iterator itr = vstmp.begin(); 
         itr != vstmp.end(); ++itr) {
      delete(*itr);
    }
    */
    return true;
  }

  bool Column::lookup_bypos(uint64_t pos, nocomp_tuple_t *v) {
    ut::_pread(fd_, rbuf, header_.tuple_size, sizeof(column_header_t) + pos * header_.tuple_size);
    memcpy(v->val, rbuf, header_.val_size);
    return true;
  }

  bool Column::lookup_bypos(uint64_t pos_start, uint64_t length, char *buf) {
    off_t read_length = length;
    if (sizeof(column_header_t) + (pos_start + length) * header_.tuple_size > header_.offset) {
      read_length = (header_.offset - sizeof(column_header_t)) / header_.tuple_size - pos_start;
    }
    return ut::_pread(fd_, buf, header_.tuple_size * read_length, sizeof(column_header_t) + pos_start * header_.tuple_size);
  }

  bool Column::lookup_bypos(uint64_t pos_start, uint64_t length, std::deque<nocomp_tuple_t *> &vs) {
    off_t read_length = length;
    if (sizeof(column_header_t) + (pos_start + length) * header_.tuple_size > header_.offset) {
      read_length = (header_.offset - sizeof(column_header_t)) / header_.tuple_size - pos_start;
    }
    ut::_pread(fd_, rbuf, header_.tuple_size * read_length, sizeof(column_header_t) + pos_start * header_.tuple_size);
    for (int i = 0; i < read_length; ++i) {
      nocomp_tuple_t *v = new nocomp_tuple_t();
      memcpy(v, rbuf + header_.tuple_size * i, header_.tuple_size);
      vs.push_back(v);
    }
    return true;
  }

  bool Column::lookup_byposrange(uint64_t pos_start, uint64_t length, std::deque<nocomp_tuple_t *> &vs) {
    off_t pos = pos_start;
    off_t pos_end = pos_start + length;
    while (true) {
      /*
      nocomp_tuple_t *v = new nocomp_tuple_t();
      bool ret = lookup_bypos(pos, v);
      if (!ret) {
        delete(v);
        break;
      }
      vs.push_back(v);
      pos++;
      */
      lookup_bypos(pos_start, length, vs);
      pos += vs.size();
      if (pos_end <= pos) {
        break;
      }
    }
    return true;
  }

  bool Column::lookup_byposlist(std::deque<off_t> &poslist, std::deque<nocomp_tuple_t *> &vs) {
    off_t start = poslist.front();
    off_t end = poslist.back();
    off_t length = end - start + 1;
    std::deque<nocomp_tuple_t *> vstmp;
    std::deque<off_t> pushed;
    lookup_byposrange(start, length, vstmp);
    for (std::deque<off_t>::iterator itr = poslist.begin(); 
         itr != poslist.end(); ++itr) {
      nocomp_tuple_t *t = vstmp.at(*itr-start);
      vs.push_back(vstmp.at(*itr-start));
      pushed.push_back(*itr - start);
    }
    std::deque<off_t>::iterator itrp = pushed.begin(); 
    for (int i = 0; i < vstmp.size(); ++i) {
      if (i == *itrp) {
        ++itrp;
      } else {
        delete(vstmp.at(i));
      }
    }
    return true;
  }

  bool Column::get_next(rle_triple_tuple_t *v, off_t *pos, void *pred, compare_t compare_type) {
    while (read_offset_ < header_.offset) {
      // TODO: error handling
      ut::_pread(fd_, rbuf, header_.tuple_size, read_offset_);
      memcpy(v->val, rbuf, header_.val_size);
      memcpy(&v->start, rbuf+header_.val_size, sizeof(off_t));
      memcpy(&v->length, rbuf+header_.val_size+sizeof(off_t), sizeof(off_t));
      read_offset_ += header_.tuple_size;
      //*pos = read_pos_++;
      *pos = v->start;
      if (pred == NULL) {
        return true;
      } else {
        switch (compare_type) {
          case EQ:
            if (generic_compare(v, pred, header_.col_type) == 0) return true;
            break;
          case NE:
            if (generic_compare(v, pred, header_.col_type) != 0) return true;
            break;
          case GT:
            if (generic_compare(v, pred, header_.col_type) > 0) return true;
            break;
          case GE:
            if (generic_compare(v, pred, header_.col_type) >= 0) return true;
            break;
          case LT:
            if (generic_compare(v, pred, header_.col_type) < 0) return true;
            break;
          case LE:
            if (generic_compare(v, pred, header_.col_type) <= 0) return true;
            break;
        }
      }
    }
    return false;
  }

  bool Column::get_next_block(std::deque<rle_triple_tuple_t *> &vs, std::deque<off_t> &poss, void *pred, compare_t compare_type, bool pos_only) {
    off_t read_size = buf_size_;
    bool ret = true;
    if (read_offset_ + read_size > header_.offset) {
      read_size = header_.offset - read_offset_;
      ret = false;
    }
    off_t num_read_tuples = read_size / header_.tuple_size;
    ut::_pread(fd_, rbuf, header_.tuple_size * num_read_tuples, read_offset_);
    read_offset_ += header_.tuple_size * num_read_tuples;
    for (int i = 0; i < num_read_tuples; ++i) {
      off_t pos = read_pos_++;
      rle_triple_tuple_t *v = new rle_triple_tuple_t();
      off_t off = header_.tuple_size * i;
      memcpy(v->val, rbuf + off, header_.val_size);
      memcpy(&v->start, rbuf + off + header_.val_size, sizeof(off_t));
      memcpy(&v->length, rbuf + off + header_.val_size + sizeof(off_t), sizeof(off_t));

      if (pred == NULL) {
        // do nothing
      } else {
        switch (compare_type) {
          case EQ:
            if (generic_compare(v->val, pred, header_.col_type) == 0) {
              break;
            } else {
              delete(v);
              continue;
            }
          case NE:
            if (generic_compare(v->val, pred, header_.col_type) != 0) {
              break;
            } else {
              delete(v);
              continue;
            }
          case GT:
            if (generic_compare(v->val, pred, header_.col_type) > 0) {
              break;
            } else {
              delete(v);
              continue;
            }
          case GE:
            if (generic_compare(v->val, pred, header_.col_type) >= 0) {
              break;
            } else {
              delete(v);
              continue;
            }
          case LT:
            if (generic_compare(v->val, pred, header_.col_type) < 0) {
              break;
            } else {
              delete(v);
              continue;
            }
          case LE:
            if (generic_compare(v->val, pred, header_.col_type) <= 0) {
              break;
            } else {
              delete(v);
              continue;
            }
        }
      }
      poss.push_back(pos);
      if (pos_only) {
        delete(v);
        continue;
      }
      vs.push_back(v);
    }
    return ret;
  }

  bool Column::get_next(nocomp_tuple_t *v, off_t *pos, void *pred, compare_t compare_type) {
    while (read_offset_ < header_.offset) {
      // TODO: error handling
      ut::_pread(fd_, rbuf, header_.tuple_size, read_offset_);
      memcpy(v->val, rbuf, header_.val_size);
      read_offset_ += header_.tuple_size;
      *pos = read_pos_++;
      if (pred == NULL) {
        return true;
      } else {
        switch (compare_type) {
          case EQ:
            if (generic_compare(v, pred, header_.col_type) == 0) return true;
            break;
          case NE:
            if (generic_compare(v, pred, header_.col_type) != 0) return true;
            break;
          case GT:
            if (generic_compare(v, pred, header_.col_type) > 0) return true;
            break;
          case GE:
            if (generic_compare(v, pred, header_.col_type) >= 0) return true;
            break;
          case LT:
            if (generic_compare(v, pred, header_.col_type) < 0) return true;
            break;
          case LE:
            if (generic_compare(v, pred, header_.col_type) <= 0) return true;
            break;
        }
      }
    }
    return false;
  }

  bool Column::get_next_block(std::deque<nocomp_tuple_t *> &vs, std::deque<off_t> &poss, void *pred, compare_t compare_type, bool pos_only) {
    off_t read_size = buf_size_;
    bool ret = true;
    if (read_offset_ + read_size > header_.offset) {
      read_size = header_.offset - read_offset_;
      ret = false;
    }
    off_t num_read_tuples = read_size / header_.tuple_size;
    ut::_pread(fd_, rbuf, header_.tuple_size * num_read_tuples, read_offset_);
    read_offset_ += header_.tuple_size * num_read_tuples;
    for (int i = 0; i < num_read_tuples; ++i) {
      // TODO: error handling
      off_t pos = read_pos_++;
      nocomp_tuple_t *v = new nocomp_tuple_t();
      memcpy(v, rbuf + header_.tuple_size * i, header_.tuple_size);
      if (pred == NULL) {
        // do nothing
      } else {
        switch (compare_type) {
          case EQ:
            if (generic_compare(v->val, pred, header_.col_type) == 0) {
              break;
            } else {
              delete(v);
              continue;
            }
          case NE:
            if (generic_compare(v->val, pred, header_.col_type) != 0) {
              break;
            } else {
              delete(v);
              continue;
            }
          case GT:
            if (generic_compare(v->val, pred, header_.col_type) > 0) {
              break;
            } else {
              delete(v);
              continue;
            }
          case GE:
            if (generic_compare(v->val, pred, header_.col_type) >= 0) {
              break;
            } else {
              delete(v);
              continue;
            }
          case LT:
            if (generic_compare(v->val, pred, header_.col_type) < 0) {
              break;
            } else {
              delete(v);
              continue;
            }
          case LE:
            if (generic_compare(v->val, pred, header_.col_type) <= 0) {
              break;
            } else {
              delete(v);
              continue;
            }
        }
      }
      poss.push_back(pos);
      if (pos_only) {
        delete(v);
        continue;
      }
      vs.push_back(v);
    }
    return ret;
  }

  bool Column::get_next(void *pred, void *v, off_t *pos) {
    while (read_offset_ < header_.offset) {
      // TODO: error handling
      ut::_pread(fd_, v, header_.tuple_size, read_offset_);
      read_offset_ += header_.tuple_size;
      *pos = read_pos_++;
      if (pred == NULL) {
        return true;
      } else {
        if (generic_compare(v, pred, header_.col_type) == 0) {
          return true;
        }
      }
    }
    return false;
  }

  bool Column::get_next(void *pred, void *v) {
    while (read_offset_ < header_.offset) {
      // TODO: error handling
      ut::_pread(fd_, v, header_.tuple_size, read_offset_);
      read_offset_ += header_.tuple_size;
      read_pos_++;
      if (pred == NULL) {
        return true;
      } else {
        if (generic_compare(v, pred, header_.col_type) == 0) {
          return true;
        }
      }
    }
    return false;
  }

  comp_t Column::get_comp_type() {
    return header_.comp_type;
  }

  col_t Column::get_col_type() {
    return header_.col_type;
  }

  off_t Column::get_offset() {
    return header_.offset;
  }

  off_t Column::get_read_offset() {
    return read_offset_;
  }

  size_t Column::get_val_size() {
    return header_.val_size;
  }

  size_t Column::get_tuple_size() {
    return header_.tuple_size;
  }

  off_t Column::get_n_tuples() {
    return header_.n_tuples;
  }

  int compare_double(Db *dbp, const Dbt *a, const Dbt *b)
  {
    double i = *((double *) a->get_data());
    double j = *((double *) b->get_data());

    if (i > j)
      return 1;
    if (i < j)
      return -1;
    return 0;
  }
  
  int compare_uint64(Db *dbp, const Dbt *a, const Dbt *b)
  {
    uint64_t i = *((uint64_t *) a->get_data());
    uint64_t j = *((uint64_t *) b->get_data());

    if (i > j)
      return 1;
    if (i < j)
      return -1;
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

  int compare_uint32(Db *dbp, const Dbt *a, const Dbt *b)
  {
    uint32_t i = *((uint32_t *) a->get_data());
    uint32_t j = *((uint32_t *) b->get_data());

    if (i > j)
      return 1;
    if (i < j)
      return -1;
    return 0;
  }

  int compare_int32(Db *dbp, const Dbt *a, const Dbt *b)
  {
    int32_t i = *((int32_t *) a->get_data());
    int32_t j = *((int32_t *) b->get_data());

    if (i > j)
      return 1;
    if (i < j)
      return -1;
    return 0;
  }
 
  int compare_uint8(Db *dbp, const Dbt *a, const Dbt *b)
  {
    uint8_t i = *((uint8_t *) a->get_data());
    uint8_t j = *((uint8_t *) b->get_data());

    if (i > j)
      return 1;
    if (i < j)
      return -1;
    return 0;
  }

  int compare_int8(Db *dbp, const Dbt *a, const Dbt *b)
  {
    int8_t i = *((int8_t *) a->get_data());
    int8_t j = *((int8_t *) b->get_data());

    if (i > j)
      return 1;
    if (i < j)
      return -1;
    return 0;
  }

  int generic_compare(void *v1, void *v2, col_t col_type)
  {
    switch (col_type) {
      case TINYINT:
        return *(int*) v1 - *(int*) v2;
      case INT:
        return *(int32_t*) v1 - *(int32_t*) v2;
      case BIGINT:
        return *(int64_t*) v1 - *(int64_t*) v2;
      case CHAR:
        return memcmp(v1, v2, 1);
      case CHAR4:
        return memcmp(v1, v2, 4);
      case CHAR8:
        return memcmp(v1, v2, 8);
      case DOUBLE:
        return *(double*) v1 - *(double*) v2;
    }
  }

  size_t get_col_size(col_t col_type) {
    switch (col_type) {
      case TINYINT:
        return 1;
      case INT:
        return 4;
      case BIGINT:
        return 8;
      case CHAR:
        return 1;
      case CHAR4:
        return 4;
      case CHAR8:
        return 8;
      case DOUBLE:
        return 8;
    }
  }
}
