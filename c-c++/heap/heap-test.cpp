#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

struct VAL {
  VAL(int val_, int off_)
  : val(val_), off(off_)
  {}
  int val;
  int off;
};

class greater_VAL : public std::binary_function<VAL, VAL, bool>
{
  public:
  result_type operator() (first_argument_type &a, second_argument_type &b)
  {
    return (result_type)((a.val > b.val) ? 1 : 0);
  }
};

int main(void)
{
  std::vector<VAL> arr;
  arr.push_back(VAL(10, 1));
  arr.push_back(VAL(30, 2));
  arr.push_back(VAL(5, 3));
  arr.push_back(VAL(3, 4));
  arr.push_back(VAL(2, 5));
  arr.push_back(VAL(100, 6));

  //make_heap(arr.begin(), arr.end(), std::greater<int>());
  make_heap(arr.begin(), arr.end(), greater_VAL());

  std::vector<VAL>::iterator itr_end = arr.end();
  for (std::vector<VAL>::iterator itr = arr.begin();
       itr != itr_end; ++itr) {
    std::cout << "val: " << itr->val << ", off: " << itr->off << std::endl;
  }

  pop_heap(arr.begin(), arr.end(), greater_VAL());
  arr.pop_back();

  std::cout << "hello" << std::endl;
  itr_end = arr.end();
  for (std::vector<VAL>::iterator itr = arr.begin();
       itr != itr_end; ++itr) {
    std::cout << "val: " << itr->val << ", off: " << itr->off << std::endl;
  }

}
