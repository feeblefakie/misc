//#include "scoped_ptr.h"
#include <tr1/memory>

class MyClass;

class Test {
public:
  Test(void);
  ~Test(void);

private:
  //Lux::scoped_ptr<MyClass> myclass;
  std::tr1::shared_ptr<MyClass> myclass;
  //MyClass *myclass;
};
