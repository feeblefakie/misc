#include <string>
#include <vector>
#include <iostream>

typedef std::vector<std::string> strs;

strs qgrams(std::string &str, uint32_t q);
off_t get_knext_offset(const unsigned char *str, size_t size, off_t off, int k = 1);

int main(void)
{
  std::string str("検索abcエンジンengine");
  strs strs = qgrams(str, 2);
  for (int i = 0; i < strs.size(); ++i) {
    std::cout << strs[i] << std::endl;
  }
  str = "engine検索";
  strs = qgrams(str, 2);
  for (int i = 0; i < strs.size(); ++i) {
    std::cout << strs[i] << std::endl;
  }

  return 0;
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
    if (from == size - 1) { break; }
  }
  return grams;
}

off_t 
get_knext_offset(const unsigned char *str, size_t size, off_t off, int k)
{
  off_t next = off;
  int cnt = 0;
  for (off_t i = off + 1; i <= size; i++) {
    if (i == size) {
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
