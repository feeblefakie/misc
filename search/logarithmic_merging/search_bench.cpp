#include <luxio/btree.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <mecab.h>
#include <time.h>
#include <sys/time.h>

typedef std::vector<uint32_t> pos_list_t;
typedef pos_list_t::iterator pos_list_itr;

std::vector<uint32_t> vb_decode_nodiff(uint8_t *bytestream, size_t size, bool skip_pos = false);
pos_list_t intersect(uint8_t *bp, uint32_t sp, uint8_t *bn, uint32_t sn, int len);
pos_list_t _intersect(uint8_t *bp, int &ip, int sp, uint8_t *bn, int &in, int sn, int len);
int32_t next(uint8_t *b, int &i, int size);
int32_t next(uint8_t *b, int &i, int size, uint32_t &prev);
void skip(uint8_t *b, int &i, int size, int skip);
int get_length_of(unsigned char *str);

double gettimeofday_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

int num_results = 0;

int main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << argv[0] << " dbname keyword" << std::endl;
    exit(1);
  }
  Lux::IO::Btree *bt = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
  bt->open(argv[1], Lux::IO::DB_RDONLY);

  std::vector<std::string> tokens;
  MeCab::Tagger *tagger = MeCab::createTagger(0, NULL);
  const MeCab::Node *node_ = tagger->parseToNode(argv[2]);
  for (; node_; node_ = node_->next) {
    if (node_->stat == MECAB_BOS_NODE ||
      node_->stat == MECAB_EOS_NODE) {
      continue; 
    }   
    std::string token(node_->surface, node_->length);
    tokens.push_back(token);
  }

  if (tokens.size() != 2) {
    std::cout << "no more than 2 tokens. exiting ..." << std::endl;
    exit(1);
  }
  std::cout << "using first two tokens" << std::endl;

  double t1 = gettimeofday_sec();
  Lux::IO::data_t *val1 = bt->get(tokens[0].c_str(), tokens[0].size());
  if (val1 == NULL) {
    std::cout << "not hit for: " << tokens[0] << std::endl;
    return 0;
  }
  /*
  std::vector<uint32_t> nums = vb_decode_nodiff((uint8_t *) val1->data, val1->size);
  int i = 0;
  for (std::vector<uint32_t>::iterator itr = nums.begin();
       itr != nums.end() && i < 100; ++itr, ++i) {
    std::cout << *itr << " ";
  }
  std::cout << std::endl;
  */
  std::cout << "val1 size: " << val1->size << std::endl;
  Lux::IO::data_t *val2 = bt->get(tokens[1].c_str(), tokens[1].size());
  if (val2 == NULL) {
    std::cout << "not hit for: " << tokens[1] << std::endl;
    return 0;
  }
  /*
  nums = vb_decode_nodiff((uint8_t *) val2->data, val2->size);
  i = 0;
  for (std::vector<uint32_t>::iterator itr = nums.begin();
       itr != nums.end() && i < 100; ++itr, ++i) {
    std::cout << *itr << " ";
  }
  std::cout << std::endl;
  */
  std::cout << "val2 size: " << val2->size << std::endl;
  double t2 = gettimeofday_sec();
  pos_list_t res = intersect((uint8_t *) val1->data, val1->size,
                             (uint8_t *) val2->data, val2->size, get_length_of((unsigned char *) tokens[0].c_str()));
  double t3 = gettimeofday_sec();
  std::cout << "search time: " << t2 - t1 << std::endl;
  std::cout << "intersection time: " << t3 - t2 << std::endl;
  std::cout << "total time: " << t3 - t1 << std::endl;
  std::cout << "num_results: " << num_results << std::endl;

  /*
  for (pos_list_itr itr = res.begin(); itr != res.end(); ++itr) {
      std::cout << *itr << " " << std::endl;
  }
  std::cout << std::endl;
  */

  bt->close();
  delete bt;

  return 0;
}

inline
std::vector<uint32_t> vb_decode_nodiff(uint8_t *bytestream, size_t size, bool skip_pos)
{
  int n = 0;
  std::vector<uint32_t> array;
  array.reserve(5*size);

  for (int i = 0; i < size; ++i) {
    n = n * 128 + bytestream[i];
    if (bytestream[i] >= 128) {
      array.push_back(n-128);
      //if (skip_pos) {
      //  skip(bytestream, i , size, next(bytestream, ++i, size));
      //}
      n = 0;
    }
  }
  return array;
}

inline 
pos_list_t intersect(uint8_t *bp, uint32_t sp,
                      uint8_t *bn, uint32_t sn, int len)
{
  pos_list_t pl;
  if (sp == 0 || sn == 0) { return pl; }
  //pl.reserve(sp > sn ? sp : sn);

  int ip = 0, in = 0;
  uint32_t prev_dp = 0, prev_dn = 0;
  int dp = next(bp, ip, sp, prev_dp);
  int dn = next(bn, in, sn, prev_dn);

  while (1) {
    if (dp == dn) {
      std::cout << "doc match: " << dp << std::endl;
      pos_list_t tp = _intersect(bp, ip, sp, bn, in, sn, len);
      if (!tp.empty()) {
        pl.push_back(dp);
        pl.push_back(tp.size());
        pos_list_itr itr_end = tp.end();
        for (pos_list_itr itr = tp.begin();
             itr != itr_end; ++itr) {
          pl.push_back(*itr);
        }
        ++num_results;
      }
      dp = next(bp, ip, sp, prev_dp);
      dn = next(bn, in, sn, prev_dn);
    } else if (dp > dn) {
      skip(bn, in, sn, next(bn, in, sn));
      dn = next(bn, in, sn, prev_dn);
    } else {
      skip(bp, ip, sp, next(bp, ip, sp));
      dp = next(bp, ip, sp, prev_dp);
    }

    if ((ip == sp && in == sn) ||
        (ip == sp && dn > dp) || 
        (in == sn && dp > dn)) {
        break;
    }
  }
  return pl;
}

// intersection between uint8_t* arrays for postions
inline pos_list_t
_intersect(uint8_t *bp, int &ip, int sp,
            uint8_t *bn, int &in, int sn, int len)
{
  pos_list_t pl;
  //pl.reserve(128); // for eliminating unnecessary copies

  int np = next(bp, ip, sp) - 1;
  int nn = next(bn, in, sn) - 1;
  int pp = next(bp, ip, sp);
  int pn = next(bn, in, sn);

  while (1) {
    if (pp + len == pn) {
      std::cout << "pp: " << pp << ", pn: " << pn << std::endl;
      pl.push_back(pp);
      if (np > 0) { pp += next(bp, ip, sp); }
      if (nn > 0) { pn += next(bn, in, sn); }
      --nn;
      --np;
    } else if (pp + len > pn) {
      if (nn > 0) { pn += next(bn, in, sn); }
      --nn;
    } else {
      if (np > 0) { pp += next(bp, ip, sp); }
      --np;
    }

    if (np < 0 && nn < 0) break;
    if (np < 0 && pn > pp + len) {
      if (nn > 0) { skip(bn, in, sn, nn); }
      break;
    }
    if (nn < 0 && pp + len > pn) {
      if (np > 0) { skip(bp, ip, sp, np); }
      break;  
    }
  }
  return pl;
}

// next number for uint8_t *
inline
int32_t next(uint8_t *b, int &i, int size)
{
  uint32_t n = 0;
  for (; i < size; ++i) {
    n = 128 * n + b[i];
    if (b[i] >= 128) {
      n -= 128;
      ++i;
      return n;
    }
  } 
  return -1;
}

inline
int32_t next(uint8_t *b, int &i, int size, uint32_t &prev)
{
  int32_t dp = next(b, i, size);
  if (dp > 0) {
    //dp += prev;
  } else if (dp == 0) {
    do {
      dp = next(b, i, size);
    } while (dp == 0);
  }
  prev = dp;
  return dp;
}

inline
void skip(uint8_t *b, int &i, int size, int skip)
{
  for (; i < size; ++i) {
    if (b[i] >= 128) {
      --skip;
      if (skip == 0) {
        ++i;
        return;
      }
    }
  } 
}

int get_length_of(unsigned char *str)
{   
  size_t str_len = strlen((char *) str);
  int ustr_len = 0;

  for (int i = 0; i < str_len; i++) {
    if (str[i] <= 0x7f || (str[i] >= 0xc0 && str[i] <= 0xfd)) {
      ustr_len++;
    }   
  }   
  return ustr_len;
}
