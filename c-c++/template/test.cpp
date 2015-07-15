#include "test.h"

int main(void)
{
  std::vector<int> nums;
  nums.push_back(10);
  nums.push_back(12);
  func(nums);

  std::deque<int> nums2;
  nums2.push_back(10);
  nums2.push_back(13);
  func(nums2);

  return 0;
}
