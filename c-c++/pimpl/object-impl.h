#include <string>

namespace Test {

struct ObjectImpl {
    explicit ObjectImpl(const char* name) : name_(name), count_(0) {}
      std::string name_;
        int         count_;
};

}
