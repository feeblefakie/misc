#include <libaio.h>
#include <iostream>
#include <linux/unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <assert.h>

#if defined(_IO) && !defined(BLKSSZGET)
#define BLKSSZGET  _IO(0x12,104)
#endif
  
#ifndef u64
typedef unsigned long long u64;
#endif
#if defined(_IO) && !defined(BLKGETSIZE64)
#define BLKGETSIZE64 _IOR(0x12,114,u64)
#endif 
  
#if defined(_IO) && !defined(BLKGETSIZE)
#define BLKGETSIZE _IO(0x12,96) /* return device size /512 (long *arg) */
#endif

static double gettimeofday_sec();
static void read_done(io_context_t ctx, struct iocb *iocb, long res, long res2);
void random_io(int fd, off_t ionum, int access_size, int num_requests);

int main(int argc, char **argv)
{
  if (argc != 5) {
    std::cerr << argv[0] << " device-name access-size access-fraction num-requests" << std::endl;
    exit(1);
  }
  char dev[128];
  sprintf(dev, "/dev/%s", argv[1]);
  int access_size = atoi(argv[2]);
  double access_fraction = atof(argv[3]);
  int num_requests = atoi(argv[4]);

  int fd = open(dev, O_RDONLY | O_DIRECT);
  if (fd < 0) {
    perror("open");
  }
  struct stat sbuf;
  if (fstat(fd, &sbuf) < 0) {
    perror("fstat");
    exit(1);
  }

  int res, t;
  res = ioctl(fd, BLKSSZGET, &t);
  off_t sz;
  res = ioctl(fd, BLKGETSIZE64, &sz);
#ifdef DEBUG
  printf("sector size: %d\n", t);
  printf("total size :%llu\n", sz);
#endif

  sz = (off_t) (sz * access_fraction);
  off_t ionum = sz / access_size;

  srand((unsigned) time(NULL));

  double t1 = gettimeofday_sec();
  random_io(fd, ionum, access_size, num_requests);
  double t2 = gettimeofday_sec();
  double iops = num_requests / (t2-t1);
  double mbps = num_requests * access_size / (t2-t1) /1024/1024;
  std::cout << access_size << " IOPS: " << iops << " MB/s: " << mbps << std::endl;

  return 0;
}

static void
read_done(io_context_t ctx, struct iocb *iocb, long res, long res2)
{
  //std::cout << "hello" << std::endl;
  //printf("%ld read\n", res);
  return;
}

void random_io(int fd, off_t ionum, int access_size, int num_requests)
{
  // (1) io_context_tの初期化
  io_context_t ctx;
  memset(&ctx, 0, sizeof(io_context_t));
  int r = io_setup(num_requests, &ctx);
  assert(r == 0);

  // (2) iocbs(I/O要求)の構築
  struct iocb **iocbs = new struct iocb*[num_requests];
  char **bufs = new char*[num_requests];
  for (int i = 0; i < num_requests; i++) {
    iocbs[i] = new struct iocb();
    posix_memalign((void **)&bufs[i], 512, access_size);

    off_t block_number = rand() % ionum;
    io_prep_pread(iocbs[i], fd, bufs[i], access_size, block_number * access_size);
    io_set_callback(iocbs[i], read_done);
  }

  // (3) I/O要求を投げる
  r = io_submit(ctx, num_requests, iocbs);
  assert(r == num_requests);

  // (4) 完了したI/O要求を待ち、終わったものについてはcallbackを呼び出す
  int cnt = 0;
  while (true) {
    struct io_event events[32];
    int n = io_getevents(ctx, 1, 32, events, NULL);
    if (n > 0)
      cnt += n;

    for (int i = 0; i < n; i++) {
      struct io_event *ev = events + i;
      io_callback_t callback = (io_callback_t)ev->data;
      struct iocb *iocb = ev->obj;
      callback(ctx, iocb, ev->res, ev->res2);
    }

    if (n == 0 || cnt == num_requests)
      break;
  }

  for (int i = 0; i < num_requests; i++) {
    delete iocbs[i];
    free(bufs[i]);
  }
  delete[] iocbs;
  delete[] bufs;
}

static double 
gettimeofday_sec()
{
  struct timeval tv; 
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}
