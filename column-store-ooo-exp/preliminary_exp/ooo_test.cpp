#include <iostream>
#include <deque>
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
#include <pthread.h>
#include <stdio.h>

#define NUM_ELEMENT 10000000

static inline bool _pread(int fd, void *buf, size_t nbyte, off_t offset);
void *lookuper(void *p);
void *putter(void *p);
static double gettimeofday_sec();

typedef struct {
  char *p;
  off_t pos; 
  off_t off;
  bool rle_encoded; 
} element_t;

typedef struct {
  int id;
  std::deque<element_t *> *q;
} thread_arg_t;

int global_cnt = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
  int num_threads = 100;

  pthread_t readers[num_threads];
  pthread_t writers[num_threads];
  thread_arg_t arg[num_threads];

  std::deque<element_t *> queues;

  double t1 = gettimeofday_sec();
  for (int i = 0; i < num_threads; ++i) {
    arg[i].id = i;
    arg[i].q = &queues;
    if (pthread_create(&writers[i], NULL, putter, (void *) &arg[i]) != 0) {
      perror("pthread_create");
      exit(1);
    }
    if (pthread_create(&readers[i], NULL, lookuper, (void *) &arg[i]) != 0) {
      perror("pthread_create");
      exit(1);
    }
  }

  void *ret = NULL;
  for (int i = 0; i < num_threads; ++i) {
    if (pthread_join(writers[i], &ret)) {
      perror("pthread_join");
    }
    if (pthread_join(readers[i], &ret)) {
      perror("pthread_join");
    }
  }


  return 0;
}

static double 
gettimeofday_sec()
{
  struct timeval tv; 
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void *lookuper(void *p)
{
  thread_arg_t *arg = (thread_arg_t *) p;
  std::deque<element_t *> *q = arg->q;

  while (true) {
    // deque
    pthread_mutex_lock(&mutex);
    if (q->empty()) {
      //usleep(100);
      pthread_mutex_unlock(&mutex);
      continue;
    }
    element_t *elem = q->front();
    delete(elem);
    q->pop_front();
    global_cnt++;
    /*
    if (global_cnt % 100000 == 0) {
      std::cout << "cnt: " << global_cnt << std::endl;
      std::cout.flush();
    }
    */
    if (global_cnt >= NUM_ELEMENT) {
      std::cout << "done" << std::endl;
      exit(1);
    }
    pthread_mutex_unlock(&mutex);
    // lookup
  }
  return NULL;
}

void *putter(void *p)
{
  thread_arg_t *arg = (thread_arg_t *) p;
  std::deque<element_t *> *q = arg->q;

  for (int i = 0; i < 100000; ++i) {
    pthread_mutex_lock(&mutex);
    // enque
    element_t *elem = new element_t();
    q->push_back(elem);
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}
