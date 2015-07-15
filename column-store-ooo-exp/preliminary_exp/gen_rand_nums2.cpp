#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <vector>

int main(int argc, char *argv[])  {

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " num" << std::endl;
    exit(1);
  }
  off_t num = atoll(argv[1]);

  std::vector<off_t> nums;
  for (off_t i = 0; i < num; ++i) {
    nums.push_back(i);
  }
  random_shuffle(nums.begin(), nums.end());

  for (off_t i = 0; i < num; ++i) {
    std::cout << nums[i] << std::endl;
  }

  return 0;
}
