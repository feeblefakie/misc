#include <set>
#include <iostream>

using namespace std;

int main()
{

  set<int> nums;

  // 要素を追加する
  nums.insert( 200 );
  nums.insert( 500 );
  nums.insert( 100 );
  nums.insert( 400 );
  nums.insert( 300 );

  // 要素を逆方向に辿る
  set<int>::reverse_iterator it = nums.rbegin();
  while( it != nums.rend() )
  {
    cout << *it << endl;
    ++it;
  }

  return 0;
}
