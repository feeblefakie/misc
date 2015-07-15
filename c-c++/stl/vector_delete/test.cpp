#include <iostream>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <algorithm>

int *deleted;

class CSample
{
  public:
    bool operator() (int num) const {
      ++cnt;
      return (deleted[num] == 1);
    }
    static int get_cnt() {
      return cnt;
    }
    static int cnt;
};
int CSample::cnt = 0;

struct Deleted : public std::unary_function<int, bool>
{
  bool operator() (int num) const {
    ++cnt;
    return (deleted[num] == 1);
  }
  static int get_cnt() {
    return cnt;
  }
  static int cnt;
};
int Deleted::cnt = 0;

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

using namespace std;

#define NUM_ITEMS 10000000

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

  deleted = new int[NUM_ITEMS];
  for (int i = 0; i < NUM_ITEMS; i++) {
    if (i % 10 == 0) {
      deleted[i] = 1;
    } else {
      deleted[i] = 0;
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
  vector<int>::iterator end_it = remove_if(array.begin(), array.end(), std::bind2nd(Deleted2(), deleted));
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
  std::cout << "cnt: " << Deleted2::get_cnt() << std::endl;

  return 0;
}
