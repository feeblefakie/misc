#include <stdio.h>
#include <libkakasi.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
  //char arg[5][4] = {"-Ha", "-Ka", "-Ja", "-Ea", "-ka"};
  char **arg = (char **) malloc(sizeof(char *) * 5);
  int i;
  for (i = 0; i < 5; ++i) {
    arg[i] = (char *) malloc(sizeof(char) * 4);
  }
  strcpy(arg[0], "-Ha");
  strcpy(arg[1], "-Ka");
  strcpy(arg[2], "-Ja");
  strcpy(arg[3], "-Ea");
  strcpy(arg[4], "-ka");

  kakasi_getopt_argv(5, (char **) arg);
  printf("kakasi_getopt_argv done\n");
  char *p = kakasi_do("hello");
  printf("kakasi_do done\n");
  kakasi_free(p);

  printf("%s\n", p);
  return 0;
}
