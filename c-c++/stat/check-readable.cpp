#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << argv[0] << " file" << std::endl; 
    exit(1);
  }

  struct stat st;
  if (stat(argv[1], &st) == -1) {
    perror("stat");
    exit(1);
  }

  if (st.st_mode & S_IRUSR) {
    std::cout << "readable" << std::endl; 
  } else {
    std::cout << "NON-readable" << std::endl; 
  }

  return 0;
}
