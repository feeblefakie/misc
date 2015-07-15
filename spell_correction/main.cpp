#include <iostream>
#include <time.h>
#include <sys/time.h>
#include "correctme.h"

double gettimeofday_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

int main(int argc, char *argv[])
{
  const char *file = "inv.idx";
  strs_map map;
  double t1 = gettimeofday_sec();
  std::cout << "mapping index ..." << std::endl;
  if (!map_index(map, file)) {
    std::cerr << "mapping index failed." << std::endl;
    exit(1);
  }
  double t2 = gettimeofday_sec();
  std::cout << "mapping done: time elapesed: " << t2 - t1 << std::endl;

  char buf[128];
  while (1) {
    memset(buf, 0, 128);
    std::cout << "keyword: ";
    scanf("%s", buf);
    double t1 = gettimeofday_sec();
    strs queries;
    queries.push_back(buf);

    strss result = correct(map, queries);
    strs list = result[0];
    std::cout << "candidates:" << std::endl;
    for (int i = 0; i < list.size(); ++i) {
      std::cout << list[i] << std::endl;
    }
    double t2 = gettimeofday_sec();
    std::cout << "correct time: " << t2 - t1 << std::endl;
    std::cout << std::endl;
  }

  return 0;
}
