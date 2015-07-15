#include <fstream>
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <libaio.h>

#define READ_SIZE 1024*100

using namespace std;

static double
gettimeofday_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

//-----------------------------------------------------------------------------
// Sync I/O
//
void
snippet1(const vector<string>& files, size_t offset)
{
  int size = files.size();
  char **bufs = new char*[size];

  for (int i = 0; i < size; i++) {
    bufs[i] = (char*)malloc(READ_SIZE);

    FILE *fp = fopen(files[i].c_str(), "rb");
    assert(fp != NULL);
    fseek(fp, offset, SEEK_SET);
    fread(bufs[i], 1, READ_SIZE, fp);
    fclose(fp);
  }

  for (int i = 0; i < size; i++) {
    /*
    cout << "----------" << endl
         << buf << endl;
    */
    free(bufs[i]);
  }
  delete[] bufs;

  return;
}

//-----------------------------------------------------------------------------
// Async I/O
//
static void
read_done(io_context_t ctx, struct iocb *iocb, long res, long res2)
{
  close(iocb->aio_fildes);
  return;
}

void
snippet2(const vector<string>& files, size_t offset)
{
  int r;
  int size = files.size();

  // (1) io_context_tの初期化
  io_context_t ctx;
  memset(&ctx, 0, sizeof(io_context_t));
  r = io_setup(size, &ctx);
  assert(r == 0);

  // (2) iocbs(I/O要求)の構築
  struct iocb **iocbs = new struct iocb*[size];
  char **bufs = new char*[size];
  for (int i = 0; i < size; i++) {
    int fd = open(files[i].c_str(), O_RDONLY);
    assert(fd >= 0);

    iocbs[i] = new struct iocb();
    bufs[i] = (char*)malloc(READ_SIZE);

    io_prep_pread(iocbs[i], fd, bufs[i], READ_SIZE, offset);
    io_set_callback(iocbs[i], read_done);
  }

  // (3) I/O要求を投げる
  r = io_submit(ctx, size, iocbs);
  assert(r == size);

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

    if (n == 0 || cnt == size)
      break;
  }

  for (int i = 0; i < size; i++) {
    /*
    cout << "----------" << endl
         << bufs[i] << endl;
    */

    delete iocbs[i];
    free(bufs[i]);
  }
  delete[] iocbs;
  delete[] bufs;
}

int
main(int argc, char **argv)
{
  if (argc != 5) {
    cout << argv[0] << " filelist maxnum offset a|s" << endl;
    return 0;
  }

  ifstream ifs(argv[1]);
  int maxnum = atoi(argv[2]);
  assert(maxnum > 0);
  size_t offset = atoi(argv[3]);

  vector<string> files;
  string line;
  while (getline(ifs, line)) {
    files.push_back(line);
    if (files.size() == maxnum)
      break;
  }

  cout << "Num: " << files.size() << endl;

  if (strcmp(argv[4], "s") == 0) {

  double t1 = gettimeofday_sec();
  snippet1(files, offset);
  double t2 = gettimeofday_sec();
  cout << "Sync  I/O Time: " << t2 - t1 << endl;

  } else {

  double t3 = gettimeofday_sec();
  snippet2(files, offset);
  double t4 = gettimeofday_sec();
  cout << "ASync I/O Time: " << t4 - t3 << endl;

  }

  return 0;
}
