#include <luxio/btree.h>
#include <iostream>

#define NUM_RECORDS 100

int main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << argv[0] << " dbname key" << std::endl;
    exit(1);
  }
  Lux::IO::Btree *bt = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
  bt->open(argv[1], Lux::IO::DB_RDONLY);

  Lux::IO::data_t *val = bt->get(argv[2], strlen(argv[2]));
  std::cout << val->size << std::endl;

  bt->close();
  delete bt;

  return 0;
}

