#include <iostream>
#include <string>
#include <tr1/unordered_set>

struct person {
  person(std::string _name, int _age)
  : name(_name), age(_age)
  {}
  person(std::string _name)
  : name(_name)
  {}
  std::string name;
  int age;
};

struct person_hash {
  std::tr1::hash<std::string> hs;
  //std::hash<int> hi;
  size_t operator()(const person& x) const {
    //return static_cast<size_t>(hs(x.name) & hi(x.age));
    return static_cast<size_t>(hs(x.name));
  }
};

struct person_equal {
  bool operator()(const person& x, const person& y) const {
    //return x.name == y.name && x.age == y.age;
    return x.name == y.name;
  }
};

typedef std::tr1::unordered_set<person,
                                person_hash,
                                person_equal> person_type;

int main(void)
{
  person_type person_set;
  person_set.insert(person("hiroyuki", 29));
  person_set.insert(person("yukiko", 31));

  person_type::iterator itr = person_set.find(person("yukiko"));

  std::cout << "name: " << itr->name << ", age: " << itr->age << std::endl;


  return 0;
}
