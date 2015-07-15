#ifndef INCLUDE_GUARD_OBJECT_H
#define INCLUDE_GUARD_OBJECT_H

namespace Test {

struct ObjectImpl;

class Object {
  public:
      explicit Object(const char* name);
        ~Object();
          void print();
//  private:
              ObjectImpl* pimpl_;
};

}

#endif // INCLUDE_GUARD_OBJECT_H
