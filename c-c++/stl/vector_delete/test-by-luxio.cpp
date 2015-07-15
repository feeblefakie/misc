#include <iostream>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <algorithm>
#include "luxio/array.h"

struct Deleted2 : public std::binary_function<int, int *, bool>
{
  bool operator() (int num, int *list) const {
    ++cnt;
    return (list[num] == 1);
  }
  static int get_cnt() {
    return cnt;
  }
  static int cnt;
};
int Deleted2::cnt = 0;

struct Deleted3 : public std::binary_function<int, void *, bool>
{
  bool operator() (int num, void *deleted) const {
    ++cnt;
    int val;
    Lux::IO::data_t val_data = {&val, 0, sizeof(int)};
    Lux::IO::data_t *val_p = &val_data;
    if (!((Lux::IO::Array *) deleted)->get(num, &val_p, Lux::IO::USER)) {
      std::cerr << "get failed" << std::endl;
      val = 0;
    }
    return (val == 1);
  }
  static int get_cnt() {
    return cnt;
  }
  static int cnt;
};
int Deleted3::cnt = 0;

using namespace std;

#define NUM_ITEMS 1000000

double gettimeofday_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

int main()
{
  double t1 = gettimeofday_sec();
  vector<int> array;
  for (int i = 0; i < NUM_ITEMS; i++) {
    array.push_back(i);
  }

  Lux::IO::Array *deleted = new Lux::IO::Array(Lux::IO::CLUSTER, sizeof(int));
  if (!deleted->open("deleted", Lux::IO::DB_CREAT)) {
    std::cerr << "can't open " << std::endl; 
    exit(1);
  }
  int val = 0;
  for (int i = 0; i < NUM_ITEMS; i++) {
    if (i % 100 == 0) {
      val = 1;
    } else {
      val = 0;
    }
    if (!deleted->put(i, &val, sizeof(int))) {
      std::cerr << "put failed" << std::endl;
    }
  }
  double t2 = gettimeofday_sec();
  std::cout << "push_backs and created deleted listr: " << t2 - t1 << std::endl;

  t1 = gettimeofday_sec();
  /*
  for (vector<int>::iterator itr = array.begin(); itr != array.end();){
    if (deleted[*(itr)] == 1) {
      itr = array.erase(itr);
    } else {
      ++itr;
    }
  }
  */
  //vector<int>::iterator end_it = remove_if(array.begin(), array.end(), CSample());
  //vector<int>::iterator end_it = remove_if(array.begin(), array.end(), Deleted());
  //vector<int>::iterator end_it = remove_if(array.begin(), array.end(), std::bind2nd(Deleted2(), deleted));
  vector<int>::iterator end_it = remove_if(array.begin(), array.end(), std::bind2nd(Deleted3(), deleted));
  t2 = gettimeofday_sec();
  std::cout << "deletion: " << t2 - t1 << std::endl;

  //for (vector<int>::iterator itr = array.begin(); itr != array.end(); ++itr) {
  /*
  for (vector<int>::iterator itr = array.begin(); itr != end_it; ++itr) {
    cout << *itr << endl;
  }
  */
  //std::cout << "cnt: " << CSample::get_cnt() << std::endl;
  //std::cout << "cnt: " << Deleted::get_cnt() << std::endl;
  //std::cout << "cnt: " << Deleted2::get_cnt() << std::endl;
  std::cout << "cnt: " << Deleted3::get_cnt() << std::endl;

  return 0;
}
