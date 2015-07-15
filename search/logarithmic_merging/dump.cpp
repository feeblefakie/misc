#include <luxio/btree.h>
#include <iostream>

#define NUM_RECORDS 100

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << argv[0] << " dbname" << std::endl;
    exit(1);
  }
  Lux::IO::Btree *bt = new Lux::IO::Btree(Lux::IO::NONCLUSTER);
  bt->open(argv[1], Lux::IO::DB_CREAT);

  Lux::IO::cursor_t *c = bt->cursor_init();
  while (bt->next(c)) {
    Lux::IO::data_t *key;
    Lux::IO::data_t *val;
    if (!bt->cursor_get(c, &key, &val, Lux::IO::SYSTEM)) {
      std::cerr << "cursor_get failed" << std::endl;
    } else {
      std::cout.write((char *) key->data, key->size);
      std::cout << std::endl;
      std::cout << val->size << std::endl;
    }
  }

  bt->cursor_fin(c);

  bt->close();
  delete bt;

  return 0;
}

