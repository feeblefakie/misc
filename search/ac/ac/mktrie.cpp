#include <fstream>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include "darts.h"

int progress_bar (size_t current, size_t total) 
{
  static char bar[] = "*******************************************";
  static int scale = sizeof(bar) - 1;
  static int prev = 0;

  int cur_percentage  = (int)(100.0 * current/total);
  int bar_len         = (int)(1.0   * current*scale/total);

  if (prev != cur_percentage) {
     printf("Making Double Array: %3d%% |%.*s%*s| ", cur_percentage, bar_len, bar, scale - bar_len, "");
     if (cur_percentage == 100)  printf("\n");
     else                        printf("\r");
     fflush(stdout);
  }
   
  prev = cur_percentage;

  return 1;
};

int main (int argc, char **argv)
{
  if (argc < 3) {
    std::cerr << "usage: " << argv[0] << " flat_file dic_name" << std::endl;
    return -1;
  }

  Darts::DoubleArray da;
  std::vector<Darts::DoubleArray::key_type *> keys;
  std::ifstream fin;
  fin.open(argv[1], std::ios::in);
  if (!fin) {
    std::cerr << "cannot open: " << argv[1] << std::endl;
    return -1;
  }

  std::string line;
  while (std::getline(fin, line)) {
    char *tmp = new char[line.size()+1];
    std::strcpy(tmp, line.c_str());
    keys.push_back(tmp);
  }
  fin.close();

  //if (da.build(keys.size(), (const Darts::DoubleArray::key_type **) &keys[0], 0, 0, &progress_bar) != 0) {
  if (da.build(keys.size(), (const Darts::DoubleArray::key_type **) &keys[0]) < 0) {
    std::cerr << "[error] cannot build trie for " << argv[1] << std::endl;
    return -1;
  }
  if (da.save(argv[2]) != 0) {
    std::cerr << "[error] cannot save trie" << std::endl;
    return -1;
  }

  for (uint32_t i = 0; i < keys.size(); i++) {
    delete [] keys[i];
  }

  std::cout << "done!, compression ratio: " << 
    100.0 * da.nonzero_size() / da.size() << " %" << std::endl;

  return 0;
}
