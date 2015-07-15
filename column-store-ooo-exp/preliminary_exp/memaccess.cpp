#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>

typedef struct {
  int id;
  int count;
  int access_count;
  int *array;
  int array_size;
} thread_arg_t;

int *array = NULL;

static double gettimeofday_sec();
void *pos_scanner(void *p);
void *pos_lookuper(void *p);

int main(int argc, char *argv[])
{
  if (argc != 4) {
    std::cerr << argv[0] << " array_size access_count concurrency" << std::endl;
    exit(1);
  }

  int array_size = atoi(argv[1]);
  int access_count = atoi(argv[2]);
  int concurrency = atoi(argv[3]);

  double t0 = gettimeofday_sec();
  array = new int[array_size];
  if (array == NULL) {
    perror("new");
    exit(1);
  }
  srand((unsigned) time(NULL));

  double t1 = gettimeofday_sec();

  pthread_t readers[concurrency];
  thread_arg_t arg[concurrency];
  if (concurrency == 1) {
    arg[0].id = 0; 
    arg[0].access_count = access_count;
    arg[0].count = access_count / array_size;
    arg[0].array_size = array_size;
    if (pthread_create(&readers[0], NULL, pos_scanner, (void *) &arg[0]) != 0) {
      perror("pthread_create");
      exit(1);
    }
  } else {
    for (int i = 0; i < concurrency; ++i) {
      arg[i].id = i;
      arg[i].count = access_count / concurrency;
      arg[i].array_size = array_size;
      if (pthread_create(&readers[i], NULL, pos_lookuper, (void *) &arg[i]) != 0) {
        perror("pthread_create");
        exit(1);
      }
    }
  }
  void *r = NULL;
  for (int i = 0; i < concurrency; ++i) {
    if (pthread_join(readers[i], &r)) {
      perror("pthread_join");
    }
  }

  double t2 = gettimeofday_sec();

  std::cout << "allocation time: " << (t1 - t0) << std::endl;
  std::cout << "processing time: " << (t2 - t1) << std::endl;

  return 0;
}

void *pos_scanner(void *p)
{
  thread_arg_t *arg = (thread_arg_t *) p;
  int count = arg->count;
  int access_count = arg->access_count;
  int array_size = arg->array_size;

  int val;
  if (access_count >= count) {
    for (off_t i = 0; i < array_size; ++i) {
      for (int j = 0; j < count; ++j) {
        val = array[i];
      }
    }
  } else {
    off_t interval = array_size / access_count;
    off_t offset = 0;
    for (int i = 0; i < access_count; ++i) {
      val = array[offset];
      offset += interval;
    }
  }
  return NULL;
}

void *pos_lookuper(void *p)
{
  thread_arg_t *arg = (thread_arg_t *) p;
  int count = arg->count;
  int size = arg->array_size;

  int val;
  for (int i = 0; i < count; ++i) {
    val = array[rand() % size];
  }
  return NULL;
}

static double 
gettimeofday_sec()
{
  struct timeval tv; 
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}
