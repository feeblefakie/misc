#include "column.h"
#include <iostream>

int main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << argv[0] << " src_column dst_column" << std::endl;
    exit(1);
  }
  int times = 100;
  char *src = argv[1];
  char *dst = argv[2];
  int fd_src;
  int fd_dst;
  cstore::column_header_t header;
  memset(&header, 0, sizeof(cstore::column_header_t));

  fd_src = ut::_open(src, COL_RDONLY, 00644);
  if (fd_src < 0) { 
    error_log("open failed.");
    return 1;
  }    

  fd_dst = ut::_open(dst, COL_CREAT, 00644);
  if (fd_dst < 0) { 
    error_log("open failed.");
    return 1;
  }    

  if (ut::_read(fd_src, &header, sizeof(cstore::column_header_t)) < 0) {
    error_log("read failed.");
    return 1;
  }

  off_t buf_size = sizeof(int32_t) * header.n_tuples;
  char *buf = new char[buf_size];
  if (buf == NULL) {
    std::cerr << "buf alloc failed" << std::endl;
    return 1;
  }
  std::cout << "copying " << header.n_tuples << " tuples x " << times << std::endl;
  std::cout << "copying " << buf_size << " bytes" << std::endl;
  header.n_tuples *= times; // x times
  header.offset = header.n_tuples * header.tuple_size + sizeof(cstore::column_header_t);

  if (!ut::_pread(fd_src, buf, buf_size, sizeof(cstore::column_header_t))) {
    error_log("read failed.");
    perror("failed");
    return 1;
  }

  off_t off = 0;
  if (!ut::_pwrite(fd_dst, &header, sizeof(cstore::column_header_t), off)) {
    error_log("read failed.");
    return 1;
  }
  off += sizeof(cstore::column_header_t);

  for (int i = 0; i < times; ++i) {
    if (!ut::_pwrite(fd_dst, buf, buf_size, off)) {
      error_log("read failed.");
      return 1;
    }
    off += buf_size;
  }
  fsync(fd_dst);
  ::close(fd_dst);

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
