#include <boost/noncopyable.hpp>

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
public:
  //constractor
  lock_free_stack() : counter(0) {
    dummy = new snode_type(0,0);
    tail = dummy;
  }

  //destractor
  ~lock_free_stack(){
    while( counter ){
      pop();
    }
  }

  //pusher
  bool push(T* in_value){
    snode_type* bk_tail;
    snode_type* new_node = new snode_type(in_value,0);
    //transaction start
    while( true ){
      bk_tail = tail;
      new_node->next = bk_tail;
      if( __sync_bool_compare_and_swap(&tail,bk_tail,new_node) ) break;
    }
    //transaction end
    __sync_add_and_fetch(&counter,1);
    return true;
  }

  //poper
  T* pop(){
    T* rtn_value;
    snode_type* bk_tail;
    //transaction start
    while( true ) {
      if(!counter) return false;
      bk_tail = tail;
      rtn_value = bk_tail->value;
      if( __sync_bool_compare_and_swap(&tail,bk_tail,tail->next) ) break;
    }
    //transaction end
    delete bk_tail;
    __sync_sub_and_fetch(&counter,1);
    return rtn_value;
  }

  //empty
  bool empty(){ return !counter; }

  //size
  unsigned int size(){ return counter; }
};
