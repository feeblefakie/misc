#include <darts.h>
#include <fstream>
#include <string>
#include <iostream>
#include <stdio.h>
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
    std::cerr << "Usage: " << argv[0] << " File Index" << std::endl;
    return -1;
  }

  std::string file  = argv[argc-2];
  std::string index = argv[argc-1];

  Darts::DoubleArray da;

  std::vector <Darts::DoubleArray::key_type *> ary;
  std::istream *is;

  if (file == "-") {
    is = &std::cin;
  } else {
    is = new std::ifstream (file.c_str());
  }

  if (! *is) {
    std::cerr << "Cannot Open: " << file << std::endl;
    return -1;
  }

  std::string line;
  while (std::getline(*is, line)) {
    char *tmp = new char [line.size()+1];
    std::strcpy (tmp, line.c_str());
    ary.push_back (tmp);
  }
  if (file != "-") delete is;

  if (da.build (ary.size(), (const Darts::DoubleArray::key_type **) &ary[0], 0, 0, &progress_bar) != 0
      || da.save (index.c_str()) != 0) {
    std::cerr << "Error: cannot build double array  " << file << std::endl;
    return -1;
  };

  for (unsigned int i = 0; i < ary.size(); i++) delete [] ary[i];

  std::cout << "Done!, Compression Ratio: " << 
    100.0 * da.nonzero_size() / da.size() << " %" << std::endl;

  return 0;
}
