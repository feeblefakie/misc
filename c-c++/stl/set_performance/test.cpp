#include <iostream>
#include <set>
//#include <hash_set>
#include <tr1/unordered_set>

template <typename SET>
void check()
{
  std::cout << typeid(SET).name() << std::endl ;

  LARGE_INTEGER now, dt, freq ;
  QueryPerformanceFrequency( &freq ) ;

  SET set ;

  QueryPerformanceCounter( &now ) ;
  for ( unsigned int i = 0 ; i != 1000000 ; ++i ) {
    set.insert(i) ;
  }
  QueryPerformanceCounter( &dt ) ;

  std::cout << "insert : " << double(dt.QuadPart - now.QuadPart) / double(freq.QuadPart) << std::endl ;

  QueryPerformanceCounter( &now ) ;
  for ( unsigned int i = 0 ; i != 1000000 ; ++i ) {
    set.find( i ) ;
  }
  QueryPerformanceCounter( &dt ) ;
  std::cout << "find : " << double(dt.QuadPart - now.QuadPart) / double(freq.QuadPart) << std::endl ;
}

int main()
{
  for ( int i = 0 ; i != 5 ; ++i )
  {
    check< std::set< unsigned int > >() ;
    //check< stdext::hash_set< unsigned int > >() ;
    check< boost::unordered_set< unsigned int > >() ;
  }
} 
