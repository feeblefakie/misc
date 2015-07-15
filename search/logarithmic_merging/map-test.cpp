#include <map>
#include <string>
#include <iostream>

using namespace std;
int main()
{

  map<int, string> names;  // キーがint、値がchar*のmap

  // 要素を追加する
  names.insert( map<int, string>::value_type( 10, "aaa" ) );
  names.insert( map<int, string>::value_type( 30, "ccc" ) );
  names.insert( map<int, string>::value_type( 50, "eee" ) );
  names.insert( map<int, string>::value_type( 40, "ddd" ) );
  names.insert( map<int, string>::value_type( 20, "bbb" ) );
  //names.insert( map<int, string>::value_type( 10, "yyy" ) );
  
  map<int, string>::iterator itr = names.find(10);
  itr->second = "yyyyyyyyy";

  // 要素を出力する
  map<int, string>::iterator it = names.begin();
  while( it != names.end() )
  {
    cout << (*it).first << ":" << (*it).second << endl;
    ++it;
  }

  // 要素数を出力する
  cout << "要素数：" << (unsigned int)names.size() << endl;

  // 要素の全削除
  names.clear();

  // 全削除されているか確認
  if( names.empty() )
  {
    cout << "空です" << endl;
  }
  return 0;
}
