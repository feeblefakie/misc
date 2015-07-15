#include "column.h"
#include <iostream>

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << argv[0] << " column" << std::endl;
    exit(1);
  }
  char *column = argv[1];
  int fd;
  cstore::column_header_t header;
  memset(&header, 0, sizeof(cstore::column_header_t));

  fd = ut::_open(column, COL_RDONLY, 00644);
  if (fd < 0) { 
    error_log("open failed.");
    return 1;
  }    

  if (ut::_read(fd, &header, sizeof(cstore::column_header_t)) < 0) {
    error_log("read failed.");
    return 1;
  }

  std::cout << "comp_type: " << header.comp_type << std::endl;
  std::cout << "col_type: " << header.col_type << std::endl;
  std::cout << "offset: " << header.offset << std::endl;
  std::cout << "val_size: " << header.val_size << std::endl;
  std::cout << "tuple_size: " << header.tuple_size << std::endl;
  std::cout << "n_tuples: " << header.n_tuples << std::endl;
  std::cout << "indexed: " << header.indexed << std::endl;

  ::close(fd);

  return 0;
}
/*
    comp_t comp_type;
    col_t col_type;
    off_t offset;
    size_t val_size;
    size_t tuple_size;
    off_t n_tuples;
    bool indexed;
*/
