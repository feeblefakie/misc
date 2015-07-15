#ifndef QUERY_UTIL_H
#define QUERY_UTIL_H

namespace cstore {

  static void output_tuple(char *v1, cstore::Column *c1, char *v2, cstore::Column *c2) {
    switch (c1->get_col_type()) {
      case cstore::TINYINT:
        std::cout << (int) v1[0];
        break;
      case cstore::INT:
        std::cout << *(int32_t *) v1;
        break;
      case cstore::BIGINT:
        std::cout << (int64_t) v1;
        break;
      case cstore::CHAR:
      case cstore::CHAR4:
      case cstore::CHAR8:
        std::cout.write(v1, c1->get_val_size());
        break;
    }
    std::cout << ",";
    switch (c2->get_col_type()) {
      case cstore::TINYINT:
        std::cout << (int) v2[0];
        break;
      case cstore::INT:
        std::cout << *(int32_t *) v2;
        break;
      case cstore::BIGINT:
        std::cout << (int64_t) v2;
        break;
      case cstore::CHAR:
      case cstore::CHAR4:
      case cstore::CHAR8:
        std::cout.write(v2, c2->get_val_size());
        break;
    }
    std::cout << std::endl;
  }

  static cstore::compare_t get_compare_type(char *compare_str) {
    if (strcmp(compare_str, "EQ") == 0) {
      return cstore::EQ;
    } else if (strcmp(compare_str, "NE") == 0) {
      return cstore::NE;
    } else if (strcmp(compare_str, "GT") == 0) {
      return cstore::GT;
    } else if (strcmp(compare_str, "GE") == 0) {
      return cstore::GE;
    } else if (strcmp(compare_str, "LT") == 0) {
      return cstore::LT;
    } else if (strcmp(compare_str, "LE") == 0) {
      return cstore::LE;
    }
  }

  static void *get_pred(char *pred_str, cstore::col_t col_type) {
    // NOTICE: currnetly assumes at most 8 bytes per attribute
    if (strcmp(pred_str, "NULL") == 0) {
      return NULL;
    }
    char *pred = new char[8];
    switch (col_type) {
      case cstore::TINYINT:
        {
          int8_t val = atoi(pred_str);
          memcpy(pred, &val, sizeof(int8_t));
          break;
        }
      case cstore::INT:
        {
          int32_t val = atoi(pred_str);
          memcpy(pred, &val, sizeof(int32_t));
          break;
        }
      case cstore::BIGINT:
        {
          int64_t val = atoll(pred_str);
          memcpy(pred, &val, sizeof(int64_t));
          break;
        }
      case cstore::CHAR:
        {
          memcpy(pred, pred_str, sizeof(char));
          break;
        }
      case cstore::CHAR4:
        {
          memcpy(pred, pred_str, sizeof(char)*4);
          break;
        }
      case cstore::CHAR8:
        {
          memcpy(pred, pred_str, sizeof(char)*8);
          break;
        }
      case cstore::DOUBLE:
        {
          double val = atof(pred_str);
          memcpy(pred, &val, sizeof(double));
          break;
        }
    }
    return pred;
  }
}

#endif
