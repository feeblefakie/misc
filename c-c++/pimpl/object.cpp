#include <iostream>
#include <string>
#include "object.h"
#include "object-impl.h"

namespace Test {

  /*
struct ObjectImpl {
    explicit ObjectImpl(const char* name) : name_(name), count_(0) {}
      std::string name_;
        int         count_;
};
*/

Object::Object(const char* name) : pimpl_(new ObjectImpl(name))
{
}

Object::~Object()
{
    delete pimpl_;
}

void Object::print()
{
    std::cout << "[" << pimpl_->count_++ << "] " << pimpl_->name_ << std::endl;
}

}
