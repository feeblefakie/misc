#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#define NODE_SIZE 4096

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "%s num_pages\n", argv[0]);
    exit(1);
  }
  int num_pages = atoi(argv[1]);

  //char *p = new char[1024*1024*1024];

  int fd = open("./test.map", O_CREAT | O_RDWR | O_TRUNC, 00644);
  if (fd < 0) {
    perror("ftruncate failed");
    exit(1);
  }

  size_t size = NODE_SIZE * num_pages;
  if (ftruncate(fd, size) < 0) {
    fprintf(stderr, "ftruncate failed.\n");
    exit(1);
  }
  void *map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  
  if (map == MAP_FAILED) {
    perror("mmap failed");
  }
  printf("mmaped\n");

  while (1) {
    /*
    void *map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  
    if (map == MAP_FAILED) {
      perror("mmap failed");
      break;
    }
    printf("mmaped\n");
    */
    /*
    char *p = (char *) map;
    for (int i = 0; i < num_pages; ++i) {
      p[0] = 1;
      p += NODE_SIZE;
    }
    */
    num_pages += 1000;
    if (ftruncate(fd, NODE_SIZE * num_pages) < 0) {
      fprintf(stderr, "ftruncate failed.\n");
      exit(1);
    }
    size_t new_size = NODE_SIZE * num_pages;
    /*
    void *map_new = mremap(map, size, new_size, MREMAP_MAYMOVE);
    if (map_new == MAP_FAILED) {
      perror("mremap failed.");
      break;
    }
    printf("mremaped\n");
    size = new_size;
    map = map_new;
    */
    if (munmap(map, size) < 0) {
      perror("munmap failed");
      break;
    }
    printf("munmaped\n");
    map = mmap(0, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  
    if (map == MAP_FAILED) {
      perror("mmap failed");
      fprintf(stderr, "new size: %d\n", new_size);
    }
    size = new_size;
    printf("mmaped\n");

    usleep(100);
  }

  close(fd);

  return 0;
}
