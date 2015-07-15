#include <cstring>

class Test {
public:
  static char *process(void)
  {
    char *buf = new char[256];
    memset(buf, 0, 256);
    strcpy(buf, "hiroyuki");
    return buf;
  }

private:
  static char *buf;
};
