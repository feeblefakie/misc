#include <luxio/btree.h>
#include <iostream>
#include <vector>

#define NUM_RECORDS 100

typedef std::vector<uint32_t> pos_list_t;
typedef pos_list_t::iterator pos_list_itr;

std::vector<uint32_t> vb_decode_nodiff(uint8_t *bytestream, size_t size, bool skip_pos = false);
pos_list_t intersect(uint8_t *bp, uint32_t sp, uint8_t *bn, uint32_t sn, int len);
pos_list_t _intersect(uint8_t *bp, int &ip, int sp, uint8_t *bn, int &in, int sn, int len);
int32_t next(uint8_t *b, int &i, int size);
//int32_t next(uint8_t *b, int &i, int size, uint32_t &prev);
int32_t next(uint8_t *b, int &i, int size, uint32_t &prev, uint32_t &gsn);
int32_t next_gsn(uint8_t *b, int &i, int size, uint32_t &prev, uint32_t &gsn);
void skip(uint8_t *b, int &i, int size, int skip);
int get_length_of(unsigned char *str);

int num_results = 0;

int main(int argc, char *argv[])
{
  if (argc != 3 && argc != 4) {
    std::cerr << argv[0] << " dbname key1 [key2]" << std::endl;
    exit(1);
  }
  Lux::IO::Btree *bt = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
  bt->open(argv[1], Lux::IO::DB_RDONLY);

  Lux::IO::data_t *val = bt->get(argv[2], strlen(argv[2]));
  if (val == NULL) {
    std::cout << "val is null" << std::endl;
    exit(1);
  }
  std::cout << val->size << std::endl;
  std::vector<uint32_t> nums = vb_decode_nodiff((uint8_t *) val->data, val->size);
  int i = 0;
  for (std::vector<uint32_t>::iterator itr = nums.begin();
       itr != nums.end() && i < 100; ++itr, ++i) {
    std::cout << *itr << " ";
  }
  std::cout << std::endl;

  if (argc == 4) { 
    Lux::IO::data_t *val2 = bt->get(argv[3], strlen(argv[3]));
    if (val2 == NULL) {
      std::cout << "val2 is null" << std::endl;
      exit(1);
    }
    /*
    std::vector<uint32_t> nums = vb_decode_nodiff((uint8_t *) val2->data, val2->size);
    int j = 0;
    for (std::vector<uint32_t>::iterator itr = nums.begin();
         itr != nums.end() && j < 100; ++itr, ++j) {
      std::cout << *itr << " ";
    }
    std::cout << std::endl;
    */
    pos_list_t res = intersect((uint8_t *) val->data, val->size,
                               (uint8_t *) val2->data, val2->size, get_length_of((unsigned char *) argv[2])); 

    /*
    for (pos_list_itr itr = res.begin(); itr != res.end(); ++itr) {
        std::cout << *itr << " " << std::endl;
    }
    std::cout << std::endl;
    */
    std::cout << "num_results: " << num_results << std::endl;
  }


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

// intersection between uint8_t* arrays for docid
inline 
pos_list_t intersect(uint8_t *bp, uint32_t sp,
                      uint8_t *bn, uint32_t sn, int len)
{
  pos_list_t pl;
  if (sp == 0 || sn == 0) { return pl; }
  //pl.reserve(sp > sn ? sp : sn);

  uint32_t gsnp, gsnn;
  int ip = 0, in = 0;
  uint32_t prev_dp = 0, prev_dn = 0;
  int dp = next(bp, ip, sp, prev_dp, gsnp);
  int dn = next(bn, in, sn, prev_dn, gsnn);

  std::cout << "before loop, gsnp:" << gsnp << ", gsnn:" << gsnn << std::endl;
  while (1) {
    if (gsnp == gsnn) {

      if (dp == dn) {
        pos_list_t tp = _intersect(bp, ip, sp, bn, in, sn, len);
        if (!tp.empty()) {
          pl.push_back(gsnp);
          pl.push_back(dp);
          pl.push_back(tp.size());
          pos_list_itr itr_end = tp.end();
          for (pos_list_itr itr = tp.begin();
               itr != itr_end; ++itr) {
            pl.push_back(*itr);
          }
          ++num_results;
        }
        dp = next(bp, ip, sp, prev_dp, gsnp);
        dn = next(bn, in, sn, prev_dn, gsnn);
      } else if (dp > dn) {
        skip(bn, in, sn, next(bn, in, sn));
        dn = next(bn, in, sn, prev_dn, gsnn);
      } else {
        skip(bp, ip, sp, next(bp, ip, sp));
        dp = next(bp, ip, sp, prev_dp, gsnp);
      }

      if ((ip == sp && in == sn) ||
          (ip == sp && dn > dp) || 
          (in == sn && dp > dn)) {
          break;
      }

    } else if (gsnp < gsnn) {
      prev_dp = 0;
      dp = next_gsn(bp, ip, sp, prev_dp, gsnp); 
    } else {
      prev_dn = 0;
      dn = next_gsn(bn, in, sn, prev_dn, gsnn); 
    }

    if (dp < 0 || dn < 0) {
      break;
    }

  }
  return pl;
}

/*
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
      pos_list_t tp = _intersect(bp, ip, sp, bn, in, sn, len);
      if (!tp.empty()) {
        pl.push_back(dp);
        pl.push_back(tp.size());
        pos_list_itr itr_end = tp.end();
        for (pos_list_itr itr = tp.begin();
             itr != itr_end; ++itr) {
          pl.push_back(*itr);
        }
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
*/

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
  //std::cout << "i: " << i << ", size: " << size << std::endl;
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

/*
inline
int32_t next(uint8_t *b, int &i, int size, uint32_t &prev)
{
  uint32_t dp = next(b, i, size);
  if (dp > 0) {
    dp += prev;
  } else if (dp == 0) {
    do {
      dp = next(b, i, size);
    } while (dp == 0);
  }
  prev = dp;
  return dp;
}
*/

inline
int32_t next(uint8_t *b, int &i, int size, uint32_t &prev, uint32_t &gsn)
{
  int32_t dp = next(b, i, size);
  if (dp > 0) {
    // for gsn program only!
    //dp += prev;
  } else if (dp == 0) {
    gsn = next(b, i, size);
    dp = next(b, i, size);
  }
  prev = dp;
  return dp;
}

inline
int32_t next_gsn(uint8_t *b, int &i, int size, uint32_t &prev, uint32_t &gsn)
{
  int32_t dp;
  do {
    //std::cout << "dp: " << dp << std::endl;
    dp = next(b, i, size);
    if (dp < 0) {
      return dp;
    }
  } while (dp != 0);
  gsn = next(b, i, size);
  dp = next(b, i, size);
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
