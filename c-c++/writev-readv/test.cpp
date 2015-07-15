#include <iostream>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
  ssize_t bytes_written;
  int fd;
  char *buf0 = "short string\n";
  char *buf1 = "This is a longer string\n";
  char *buf2 = "This is the longest string in this example\n";
  int iovcnt;
  struct iovec iov[3];

  fd = open("test.out", O_CREAT | O_RDWR | O_TRUNC, 00644);
  if (fd < 0) {
    std::cerr << "open failed." << std::endl;
    exit(1);
  }

  iov[0].iov_base = buf0;
  std::cout << "len: " << strlen(buf0) << std::endl;
  iov[0].iov_len = strlen(buf0);
  iov[1].iov_base = buf1;
  std::cout << "len: " << strlen(buf1) << std::endl;
  iov[1].iov_len = strlen(buf1);
  iov[2].iov_base = buf2;
  std::cout << "len: " << strlen(buf2) << std::endl;
  iov[2].iov_len = strlen(buf2);
  iovcnt = sizeof(iov) / sizeof(struct iovec);

  bytes_written = writev(fd, iov, iovcnt);
  std::cout << "bytes_written: " << bytes_written << std::endl;

  close(fd);

  return 0;
}
