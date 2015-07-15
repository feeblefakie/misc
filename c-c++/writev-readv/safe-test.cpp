#include <iostream>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

ssize_t my_writev(int fd, struct iovec *vector, int count);
ssize_t test_writev(int fd, const struct iovec *vector, int count);

int main(void)
{
  ssize_t bytes_written;
  int fd;
  char *buf0 = "short string\n";
  char *buf1 = "This is a longer string\n";
  char *buf2 = "This is the longest string in this example";
  int iovcnt;
  struct iovec iov[3];

  fd = open("test.out", O_CREAT | O_RDWR | O_TRUNC, 00644);
  if (fd < 0) {
    std::cerr << "open failed." << std::endl;
    exit(1);
  }

  iov[0].iov_base = buf0;
  //std::cout << "len: " << strlen(buf0) << std::endl;
  iov[0].iov_len = strlen(buf0);
  iov[1].iov_base = buf1;
  //std::cout << "len: " << strlen(buf1) << std::endl;
  iov[1].iov_len = strlen(buf1);
  iov[2].iov_base = buf2;
  //std::cout << "len: " << strlen(buf2) << std::endl;
  iov[2].iov_len = strlen(buf2);
  iovcnt = sizeof(iov) / sizeof(struct iovec);

  bytes_written = my_writev(fd, iov, iovcnt);
  //std::cout << "bytes_written: " << bytes_written << std::endl;

  close(fd);

  return 0;
}

ssize_t
my_writev(int fd, struct iovec *vector, int count)
{
  size_t total = 0;
  for (int i = 0; i < count; ++i) {
    total += vector[i].iov_len;
  }

  size_t remain = total;
  while (1) {
    ssize_t num_bytes = test_writev(fd, vector, count);
    remain -= num_bytes;
    if (!remain) { break; }

    for (int i = 0; i < count; ++i) {
      if (num_bytes >= vector[i].iov_len) {
        num_bytes -= vector[i].iov_len;
      } else {
        vector[i].iov_len -= num_bytes;
        char *p = (char *) vector[i].iov_base;
        p += num_bytes;
        vector[i].iov_base = p;
        int j = 0;
        for (; i < count && j < count; ++i, ++j) {
          vector[j] = vector[i];
        }
        count = j;
      }
    }
  }

  return total;
}

ssize_t
test_writev(int fd, const struct iovec *vector, int count)
{
  size_t total = 0;
  for (int i = 0; i < count; ++i) {
    total += vector[i].iov_len;
  }
  if (total > 30) {
    return 30;
  }
  return total;
}
