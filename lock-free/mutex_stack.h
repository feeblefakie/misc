#include <boost/noncopyable.hpp>
#include <pthread.h>

template <class T>
class lock_free_stack : protected boost::noncopyable {
protected:
  struct snode_type{
    T*   value;
    snode_type* next;
    snode_type(T* nin_value,snode_type* nin_next) :
      value( nin_value ) , next( nin_next ) {}
  };
  snode_type*  tail;
  snode_type*  dummy;
  unsigned int counter;
  pthread_mutex_t mutex;
public:
  //constractor
  lock_free_stack() : counter(0) {
    dummy = new snode_type(0,0);
    tail = dummy;
    pthread_mutex_init(&mutex, NULL);
  }

  //destractor
  ~lock_free_stack(){
    while( counter ){
      pop();
    }
  }

  //pusher
  bool push(T* in_value){
    pthread_mutex_lock(&mutex);
    snode_type* new_node = new snode_type(in_value,tail);
    tail = new_node;
    counter++;
    pthread_mutex_unlock(&mutex);
    return true;
  }

  //poper
  T* pop(){
    T* rtn_value;
    snode_type* bk_tail = tail;
    if(!counter) return false;
    rtn_value = tail->value;
    tail = tail->next;
    delete bk_tail;
    counter--;
    return rtn_value;
  }

  //empty
  bool empty(){ return !counter; }

  //size
  unsigned int size(){ return counter; }
};
