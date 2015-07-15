#include <iostream>
#include <string>

int main(void)
{
  char *str1 = "hiroyuki";
  char *str2 = "hiroyuking";

  int n = strncmp(str1, str2, strlen(str1));
  printf("%d\n", n);

  return 0;
}
