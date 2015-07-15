#include <iostream>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 300000000

int main(void)
{
  ssize_t bytes_written;
  int fd;
  char *buf0 = new char[SIZE];
  memset(buf0, 0, SIZE);
  char *buf1 = new char[SIZE];
  memset(buf1, 0, SIZE);
  char *buf2 = new char[SIZE];
  memset(buf2, 0, SIZE);
  int iovcnt;
  struct iovec iov[3];

  fd = open("test.out", O_CREAT | O_RDWR | O_TRUNC, 00644);
  if (fd < 0) {
    std::cerr << "open failed." << std::endl;
    exit(1);
  }

  iov[0].iov_base = buf0;
  iov[0].iov_len = SIZE;
  iov[1].iov_base = buf1;
  iov[1].iov_len = SIZE;
  iov[2].iov_base = buf2;
  iov[2].iov_len = SIZE;
  iovcnt = sizeof(iov) / sizeof(struct iovec);
  std::cout << "total writing: " << SIZE * 3 << std::endl;

  bytes_written = writev(fd, iov, iovcnt);
  std::cout << "bytes_written: " << bytes_written << std::endl;

  close(fd);

  return 0;
}
