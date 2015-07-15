#include <iostream>
#include <exception>
#include <string>

class luxio_error : public std::exception {
public:
  explicit luxio_error(const std::string &msg)
  : std::exception(), msg_(msg)
  {}
  virtual ~luxio_error() throw () 
  {}
  virtual const char * what() const throw()
  {
    return msg_.c_str();
  }
private:
  std::string msg_;
};


class mmap_alloc_error : public luxio_error {
public:
  explicit mmap_alloc_error(const std::string &msg)
  : luxio_error("[mmap_alloc_error] " + msg)
  {}
  virtual ~mmap_alloc_error() throw () 
  {}
};  

class disk_alloc_error : public luxio_error {
public:
  explicit disk_alloc_error(const std::string &msg)
  : luxio_error("[disk_alloc_error] " + msg)
  {}
  virtual ~disk_alloc_error() throw () 
  {}
};

void func(void);

int main(void)
{
  try {
    func();
  } catch (const mmap_alloc_error &e) {
    std::cerr << "catched by mmap_alloc_error: " << e.what() << std::endl;
  } catch (const disk_alloc_error &e) {
    std::cerr << "catched by disk_alloc_error: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "catched by exception: " << e.what() << std::endl;
  }
  return 0;
}

void func(void)
{
  //try {
    throw disk_alloc_error("ftruncate failed.");
  //} catch (const disk_alloc_error &e) {
  //  throw std::exception();
  //}
}
