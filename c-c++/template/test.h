#include <iostream>
#include <vector>
#include <deque>

template <typename Container>
struct get_iterator
{
  typedef typename Container::iterator type;
};
//static inline
template <typename T>
void func__(T &numbers)
{
  get_iterator<T>::type itr = numbers.begin();
  for (; itr != numbers.end(); ++itr) {
    std::cout << *itr << std::endl;
  }
}

static inline
void func(std::vector<int> &numbers)
{
  func__< std::vector<int> >(numbers);
}

static inline
void func(std::deque<int> &numbers)
{
  func__< std::deque<int> >(numbers);
}
