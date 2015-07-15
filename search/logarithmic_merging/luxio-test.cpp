#include <luxio/btree.h>
#include <iostream>

#define NUM_RECORDS 100

int main(void)
{
  Lux::IO::Btree *bt = new Lux::IO::Btree(Lux::IO::CLUSTER);
  bt->open("test", Lux::IO::DB_CREAT);

  char str[9];
  for (int i = 0; i < NUM_RECORDS; ++i) {
    sprintf(str, "%08d", i);

    Lux::IO::data_t key = {str, strlen(str)};
    Lux::IO::data_t val = {&i, sizeof(int)};
    bt->put(&key, &val); // insert operation
  }
  
  Lux::IO::cursor_t *c = bt->cursor_init();
  while (bt->next(c)) {
    Lux::IO::data_t *key;
    Lux::IO::data_t *val;
    if (!bt->cursor_get(c, &key, &val, Lux::IO::SYSTEM)) {
      std::cerr << "cursor_get failed" << std::endl;
    } else {
      std::cout.write((char *) key->data, key->size);
      std::cout << std::endl;
    }
  }

  bt->cursor_fin(c);


  bt->close();
  delete bt;

  return 0;
}

