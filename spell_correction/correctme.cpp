#include "correctme.h"

bool
map_index(strs_map &map, const char *index_file)
{
  std::string l;
  std::ifstream ifs(index_file);
  while (getline(ifs, l)) {
    char *line = const_cast<char *>(l.c_str());
    // split "gram\tkeys"
    char *keys_str = strstr(line, GRAM_SEP);
    if (keys_str == NULL) { continue; }   
    *keys_str++ = '\0';
    // split "keykey...key"
    strs keys;
    char *saveptr;
    for (char *key = strtok_r(keys_str, KEYS_SEP, &saveptr);
         key; 
         key = strtok_r(NULL, KEYS_SEP, &saveptr)) {
      keys.push_back(std::string(key));
    }
    map.insert(
      std::pair< std::string, strs >
        (std::string(line), keys)
    );
  }
  return true;
}

uint32_t
edit_distance(const std::string &s1, const std::string &s2)
{
  size_t len1 = s1.size();
  size_t len2 = s2.size();
  uint32_t d[len1+1][len2+1];

  d[0][0] = 0;
  for(uint32_t i = 1; i <= len1; ++i) { d[i][0] = i; }
  for(uint32_t i = 1; i <= len2; ++i) { d[0][i] = i; }

  for(uint32_t i = 1; i <= len1; ++i) {
    for(uint32_t j = 1; j <= len2; ++j) {
      d[i][j] = std::min(
        std::min(d[i - 1][j] + 1,d[i][j - 1] + 1), 
                 d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1)
     );
    }
  }
  return d[len1][len2];
}

strss
correct(strs_map &map, strs &queries)
{
  strss results;
  strs_iterator itr_end = queries.end();
  for (strs_iterator q_itr = queries.begin();
       q_itr != itr_end; ++q_itr) {
    strs result;
    int_map counter;
    strs grams = qgrams(*q_itr, Q);
    strs_iterator grams_itr_end = grams.end();
    for (strs_iterator grams_itr = grams.begin();
         grams_itr != grams_itr_end; ++grams_itr) {
      strs_map_iterator strs_map_itr = map.find(*grams_itr);
      if (strs_map_itr == map.end()) { continue; }

      strs_iterator keys_itr_end = strs_map_itr->second.end();
      for (strs_iterator keys_itr = strs_map_itr->second.begin();
           keys_itr != keys_itr_end; ++keys_itr) {
        int_map_iterator int_map_itr = counter.find(*keys_itr);
        if (int_map_itr == counter.end()) {
          counter.insert(value_type(*keys_itr, 1));
        } else {
          const_cast<uint32_t &>(int_map_itr->second)++;
        }
      }
    }

    std::priority_queue<value_type,
                        std::vector<value_type>,
                        second_descend> gramq;
    int_map_iterator int_map_itr_end = counter.end();
    for (int_map_iterator int_map_itr = counter.begin();
         int_map_itr != int_map_itr_end; ++int_map_itr) {
      gramq.push(*int_map_itr);
    }

    std::priority_queue<value_type,
                        std::vector<value_type>,
                        second_ascend> distq;
    while (!gramq.empty()) {
      value_type item = gramq.top();
      if (item.second <= 1) {
        break;
      }
      double jaccard = (double) item.second / 
        (q_itr->size() - 1 + item.first.size() - 1 - item.second);
      uint32_t dist = edit_distance(*q_itr, item.first);
      distq.push(value_type(item.first, dist));
      gramq.pop();
      if (jaccard < JACCARD_THRESHOLD) { continue; }
    }

    int i = 0;
    while (!distq.empty()) {
      value_type item = distq.top();
      //std::cout << item.first << " [" << item.second << "]" << std::endl;
      result.push_back(item.first);
      distq.pop();
      if (++i == NUM_RESULT) {
        break;
      }
    }
    results.push_back(result);
  }
  return results;
}

strs
qgrams(std::string &str, uint32_t q)
{
  strs grams;
  size_t size = str.size();
  const unsigned char *c_str = (const unsigned char *) str.c_str();
  grams.reserve(size-q+1);
  off_t from = 0;
  while (1) {
    off_t to = get_knext_offset(c_str, size, from, q);
    if (to == from) { break; }
    grams.push_back(str.substr(from, to-from+1));
    from = get_knext_offset(c_str, size, from) + 1;
    if ((size_t) from == size - 1) { break; }
  }
  return grams;
}

off_t 
get_knext_offset(const unsigned char *str, size_t size, off_t off, int k)
{
  off_t next = off;
  int cnt = 0;
  for (off_t i = off + 1; i <= (off_t) size; i++) {
    if (i == (off_t) size) {
      if (++cnt == k) {
        next = i -1;
      }
      break;
    }
    if (str[i] <= 0x7f || (str[i] >= 0xc0 && str[i] <= 0xfd)) {
      if (++cnt == k) {
        next = i-1;
        break;
      }
    }
  }
  return next;
}
