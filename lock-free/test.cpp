#include "lock_free_stack.h"
//#include "mutex_stack.h"
#include <iostream>
#include <vector>
#include <pthread.h>

lock_free_stack< long long > nstack;

void*thread_func(void* param) {
  for(long long i = 0 ; i < 10000; ++i){
    nstack.push( &i );
  }
  /*
  for( long long i = 0 ; i < 100; ++i ){
    long long* value = nstack.pop();
  }
  */
  return 0;
}

int main(void)
{
  std::vector<pthread_t> thread_vec;
  pthread_t thd;

  for( int i = 0; i < 100; ++i ){
    pthread_create( &thd, NULL, thread_func, NULL );
    thread_vec.push_back( thd );
  }
  for( std::vector<pthread_t>::iterator itr = thread_vec.begin();
       itr != thread_vec.end();
       ++itr ){
    pthread_join( *itr, NULL );
  }
  thread_vec.clear();

  return 0;
}
