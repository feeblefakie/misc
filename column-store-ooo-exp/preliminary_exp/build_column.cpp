#include <iostream>
#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "column.h"

void append_nocomp(cstore::Column *c, std::string attr, cstore::col_t col_type);
void append_rle(cstore::Column *c, std::string attr, off_t start, off_t length, cstore::col_t col_type, bool only_idx_print=false);
void get_typed_val(std::string val_str, char *val, cstore::col_t col_type);

int main(int argc, char *argv[]) {

  if (argc < 6) {
    std::cerr << "Usage: " << argv[0] << " file('|'-separated) offset(0-base) column_name column_type(TINYINT|INT|BIGINT|CHAR|CHAR4|CHAR8|DOUBLE) compression(no|rle) indexing(y|n) only_idx_print(only RLE)" << std::endl;
    exit(1);
  }
  char *file = argv[1];
  int rec_off = atoi(argv[2]);
  char *col_name = argv[3];
  char *col_t = argv[4];
  char *comp = argv[5];
  char *index = "n";
  bool only_idx_print = false;
  if (argc == 7) {
    index = argv[6];
  }
  if (argc == 8) {
    only_idx_print = true;
  }

  cstore::comp_t comp_type;
  cstore::col_t col_type;
  size_t col_size;
  bool indexing = false; 

  if (strcmp(comp, "rle") == 0) {
    comp_type = cstore::RLE;
  } else {
    comp_type = cstore::NOCOMP;
  }

  if (strcmp(index, "y") == 0) {
    indexing = true;
  }

  if (strcmp(col_t, "TINYINT") == 0) {
    col_type = cstore::TINYINT;
    col_size = 1;
  } else if (strcmp(col_t, "INT") == 0) {
    col_type = cstore::INT;
    col_size = 4;
  } else if (strcmp(col_t, "BIGINT") == 0) {
    col_type = cstore::BIGINT;
    col_size = 8;
  } else if (strcmp(col_t, "CHAR") == 0) {
    col_type = cstore::CHAR;
    col_size = 1;
  } else if (strcmp(col_t, "CHAR4") == 0) {
    col_type = cstore::CHAR4;
    col_size = 4;
  } else if (strcmp(col_t, "CHAR8") == 0) {
    col_type = cstore::CHAR8;
    col_size = 8;
  } else if (strcmp(col_t, "DOUBLE") == 0) {
    col_type = cstore::DOUBLE;
    col_size = 8;
  }

  cstore::Column *c = new cstore::Column();
  c->set_column_type(comp_type, col_type);
  c->open(col_name, COL_CREAT, indexing);

  std::ifstream ifs(file);
  if (!ifs) {
    std::cerr << "can't open " << file << std::endl;
    exit(1);
  }

  std::string line;
  char val[col_size];
  off_t start = 0;
  off_t pos = 0;
  std::string prev_attr;
  while (getline(ifs, line)) {
    if (line.find_first_of("#") == 0) { continue; }
    std::vector<std::string> attrs;
    //boost::split(attrs, line, boost::is_any_of("|"));
    boost::split(attrs, line, boost::is_any_of(","));
    std::string attr = boost::algorithm::replace_all_copy(attrs.at(rec_off), "-", "");
    if (attr.empty()) { attr = "-1"; }
    
    if (comp_type == cstore::RLE) {
      if (!prev_attr.empty() && prev_attr != attr) {
        // append w/ start, pos-start(length)
        append_rle(c, prev_attr, start, pos-start, col_type, only_idx_print);
        start = pos;
      }
    } else {
        append_nocomp(c, attr, col_type);
    }
    pos++;
    prev_attr = attr;
    if (pos % 1000000 == 0) {
      std::cerr << "loaded " << pos << " values" << std::endl;
    }
  }

  if (comp_type == cstore::RLE && start != pos) {
    append_rle(c, prev_attr, start, pos-start, col_type, only_idx_print);
  }

  delete(c);

  return 0;
}

// TODO: should use polymorphism, but just switching logic by colunm type for now
void append_nocomp(cstore::Column *c, std::string attr, cstore::col_t col_type) {
  //std::cout << "appending attr: " << attr << std::endl;
  switch (col_type) {
    case cstore::TINYINT:
      {
        int8_t tmp = atoi(attr.c_str()); 
        cstore::nocomp_val1_t v;
        v.val[0] = tmp;
        c->append(&v);
        break;
      }
    case cstore::INT:
      {
        int32_t tmp = atoi(attr.c_str()); 
        cstore::nocomp_val4_t v;
        memcpy(v.val, &tmp, sizeof(v.val));
        c->append(&v);
        break;
      }
    case cstore::BIGINT:
      {
        int64_t tmp = atoll(attr.c_str()); 
        cstore::nocomp_val8_t v;
        memcpy(v.val, &tmp, sizeof(v.val));
        c->append(&v);
        break;
      }
    case cstore::CHAR:
      {
        cstore::nocomp_val1_t v;
        v.val[0] = attr.at(0);
        c->append(&v);
        break;
      }
    case cstore::CHAR4:
      {
        cstore::nocomp_val4_t v;
        memcpy(v.val, attr.c_str(), sizeof(v.val));
        c->append(&v);
        break;
      }
    case cstore::CHAR8:
      {
        cstore::nocomp_val8_t v;
        memcpy(v.val, attr.c_str(), sizeof(v.val));
        c->append(&v);
        break;
      }
    case cstore::DOUBLE:
      {
        double tmp = atof(attr.c_str()); 
        cstore::nocomp_val8_t v;
        memcpy(v.val, &tmp, sizeof(v.val));
        c->append(&v);
        break;
      }
  }
}

// TODO: should use polymorphism, but just switching logic by colunm type for now
void append_rle(cstore::Column *c, std::string attr, off_t start, off_t length, cstore::col_t col_type, bool only_idx_print) {
  static off_t off = 0;
  if (only_idx_print) {
    off += length - 1;
    std::cout << attr << " " << off << std::endl;
    off++;
    return;
  }
  //std::cout << "appending attr: " << attr << ", start: " << start << ", length: " << length << std::endl;
  switch (col_type) {
    case cstore::TINYINT:
      {
        int8_t tmp = atoi(attr.c_str()); 
        cstore::rle_triple_val1_t v;
        v.val[0] = tmp;
        v.start = start;
        v.length = length;
        c->append(&v);
        break;
      }
    case cstore::INT:
      {
        int32_t tmp = atoi(attr.c_str()); 
        cstore::rle_triple_val4_t v;
        memcpy(v.val, &tmp, sizeof(v.val));
        v.start = start;
        v.length = length;
        c->append(&v);
        break;
      }
    case cstore::BIGINT:
      {
        int64_t tmp = atoll(attr.c_str()); 
        cstore::rle_triple_val8_t v;
        memcpy(v.val, &tmp, sizeof(v.val));
        v.start = start;
        v.length = length;
        c->append(&v);
        break;
      }
    case cstore::CHAR:
      {
        cstore::rle_triple_val1_t v;
        v.val[0] = attr.at(0);
        v.start = start;
        v.length = length;
        c->append(&v);
        break;
      }
    case cstore::CHAR4:
      {
        cstore::rle_triple_val4_t v;
        memcpy(v.val, attr.c_str(), sizeof(v.val));
        v.start = start;
        v.length = length;
        c->append(&v);
        break;
      }
    case cstore::CHAR8:
      {
        cstore::rle_triple_val8_t v;
        memcpy(v.val, attr.c_str(), sizeof(v.val));
        v.start = start;
        v.length = length;
        c->append(&v);
        break;
      }
    case cstore::DOUBLE:
      {
        double tmp = atof(attr.c_str()); 
        cstore::rle_triple_val8_t v;
        memcpy(v.val, &tmp, sizeof(v.val));
        v.start = start;
        v.length = length;
        c->append(&v);
        break;
      }
  }
}

// TODO: should use polymorphism, but just switching logic by colunm type for now
void get_typed_val(std::string attr, char *val, cstore::col_t col_type) {
  switch (col_type) {
    case cstore::TINYINT:
      {
        int8_t tmp = atoi(attr.c_str()); 
        val[0] = tmp;
        break;
      }
    case cstore::INT:
      {
        int32_t tmp = atoi(attr.c_str()); 
        memcpy(val, &tmp, 4);
        break;
      }
    case cstore::BIGINT:
      {
        int64_t tmp = atoll(attr.c_str()); 
        memcpy(val, &tmp, 8);
        break;
      }
    case cstore::CHAR:
      memcpy(val, attr.c_str(), 1);
      break;
    case cstore::CHAR4:
      memcpy(val, attr.c_str(), 4);
      break;
    case cstore::CHAR8:
      memcpy(val, attr.c_str(), 8);
      break;
    case cstore::DOUBLE:
      {
        double tmp = atof(attr.c_str()); 
        memcpy(val, &tmp, 8);
        break;
      }
  }

}
