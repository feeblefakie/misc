#include "test.h"

int main(void)
{
  MemberFunc pfunc = &Test::func;
  Test *p;

  //(p->*pfunc)();
  pfunc();

  return 0;
}
