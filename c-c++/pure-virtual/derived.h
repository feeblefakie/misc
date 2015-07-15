#include "test.h"

class Derived : public TestBase {
public:
  Derived(void);
  virtual ~Derived(void);

protected:
  virtual void exec(void);
};
