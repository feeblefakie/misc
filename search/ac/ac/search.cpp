#include <iostream>
#include "darts.h"

int main(int argc, char **argv)
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " darts_file" << std::endl;
    return -1;
  }

  Darts::DoubleArray da;
  if (da.open(argv[1]) != 0) {
    std::cerr << "error: can't open " << argv[1] << std::endl;
    return -1;
  }

  Darts::DoubleArray::result_pair_type daresult[512];
  std::string line;
  while (std::getline(std::cin, line)) {

    const char *text = line.c_str();
    const char *begin = text;
    const char *end   = text + strlen(text);

    while (begin < end) {
      size_t size = da.commonPrefixSearch(begin, daresult, 512, (size_t)(end - begin));
      size_t seekto = 0;

      if (size) {
        std::cout << "size: " << size << std::endl;
        // finding the longest match ?
        for (size_t i = 0; i < size; ++i) {
          if (seekto < daresult[i].length) {
            seekto = daresult[i].length;
          }
        }
	 
        if (seekto) {
          std::cout << "#";
          std::cout.write(begin, seekto);
          std::cout << "#" << std::endl;
          begin += seekto;
        }
      }
       
      if (seekto == 0) {
        std::cout.write(begin, 1);
        ++begin; 
      }
    }
    std::cout << std::endl;
  }
}
