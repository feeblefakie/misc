#include <stdio.h>

extern "C" {

int lookup(int a)
{
  int result = 10;
  printf("looking up %d\n", a);
  return result;
}

}
