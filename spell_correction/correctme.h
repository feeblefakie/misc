#include <fstream>
#include <vector>
#include <string>
#include <queue>
//#include <tr1/unordered_map>
#include <ext/hash_map>
#include <map>
#include <boost/algorithm/string.hpp>

#define GRAM_SEP "\t"
#define KEYS_SEP ""
#define Q 2
#define JACCARD_THRESHOLD 0.4
#define NUM_RESULT 10

typedef std::vector<std::string> strs;
typedef strs::iterator strs_iterator;
typedef std::vector<strs> strss;
//typedef std::tr1::unordered_map<std::string, strs> strs_map;
typedef std::map<std::string, strs> strs_map;
typedef strs_map::iterator strs_map_iterator;
//typedef std::tr1::unordered_map<std::string, uint32_t> int_map;
typedef std::map<std::string, uint32_t> int_map;
typedef int_map::iterator int_map_iterator;
typedef std::pair<std::string, uint32_t> value_type;

struct second_descend {
  bool operator() (const value_type& x, const value_type& y) const {
    return x.second < y.second;
  }
};
struct second_ascend {
  bool operator() (const value_type& x, const value_type& y) const {
    return x.second > y.second;
  }
};

struct gram_keys {
  std::string gram;
  std::vector<std::string> keys;
};  

template <class T>
struct less_gram : public std::binary_function <T, T, bool> {
  bool operator() (const T& t1, const T& t2) const {
    return t1.gram < t2.gram;
  }
};

bool map_index(strs_map &map, const char *index_file);
uint32_t edit_distance(const std::string &s1, const std::string &s2);
strss correct(strs_map &map, strs &queries);
strs qgrams(std::string &str, uint32_t q);
off_t get_knext_offset(const unsigned char *str, size_t size, off_t off, int k = 1);
