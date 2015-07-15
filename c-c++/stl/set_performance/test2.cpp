#include <set>
#include <string>
#include <iostream>
//#include <functional>

template <class T>    
struct less_index_to : public std::binary_function <T, T, bool> {
  bool operator() (const T& t1, const T& t2) const {
    return t1.index_to < t2.index_to;
  }
};

template <class T>    
struct less_term : public std::binary_function <T, T, bool> {
  bool operator() (const T& t1, const T& t2) const {
    return t1.term < t2.term;
  }   
};  

struct TermBuffer {
  TermBuffer(std::string term_)
  : term(term_)
  {}  
  std::string term;
};  

typedef std::set< TermBuffer, less_term<TermBuffer> > TermBufferSet;
typedef TermBufferSet::iterator TBIterator;

/**
 * IndexBuffer
 */
struct IndexBuffer {
  IndexBuffer(std::string index_to_)
  : index_to(index_to_)
  {}
  std::string index_to;
  TermBufferSet term_buffer_set;
};
typedef std::set< IndexBuffer, less_index_to<IndexBuffer> > IBSet;
typedef IBSet::iterator IBIterator;

/**
 * class IndexBufferSet
 */
class IndexBufferSet {

public:
  IndexBufferSet()
  : curr_buffer_size_(0)
  {
    IndexBuffer index_buffer("default");
    ib_set_.insert(index_buffer);
  }

  // merge with TermBufferSet for index_to
  void merge(std::string &index_to, TermBufferSet &term_buffer_set)
  {
    IBIterator ib_itr = ib_set_.find(IndexBuffer(index_to));
    TBIterator tb_itr_end = term_buffer_set.end();
    for (TBIterator tb_itr = term_buffer_set.begin();
         tb_itr != tb_itr_end; ++tb_itr) {
      
      std::cout << "DEBUG: [" << tb_itr->term << "]" << std::endl;

      TBIterator tb_itr2 = const_cast<TermBufferSet&>(ib_itr->term_buffer_set).find(*tb_itr);
      if (tb_itr2 == ib_itr->term_buffer_set.end()) {
        std::cout << "end!" << std::endl;
      }
    }
  }

private:
  IBSet ib_set_;
  IBIterator ib_itr_;
  int curr_buffer_size_;
};

int main(void)
{
  IndexBufferSet *ibs = new IndexBufferSet();
  TermBufferSet tbset;

  TermBuffer t1("a");
  TermBuffer t2("b");
  TermBuffer t3("c");

  tbset.insert(t1);
  tbset.insert(t2);
  tbset.insert(t3);

  std::string def("default");
  ibs->merge(def, tbset);

  TBIterator itr = tbset.find(t2);
  if (itr == tbset.end()) {
    std::cerr << "error" << std::endl;
  }

  return 0;
}
