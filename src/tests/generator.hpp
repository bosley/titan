#ifndef GENERATOR_TESTS_HPP
#define GENERATOR_TESTS_HPP

#include "types/types.hpp"

#include <random>
#include <string>

namespace gen
{

template<typename T>
class GenerateRandom
{
public:
    explicit GenerateRandom() : eng(rd()) { }
    T get_range(T min, T max)
    {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(eng);
    }
private:
    std::random_device rd;
    std::default_random_engine eng;
};


template<class T>
class RandomEntry
{
public:
    explicit RandomEntry(std::vector<T> values) : _values(values) { }
    T get_value()
    {
        GenerateRandom<int64_t> r;
        auto idx = r.get_range(0, _values.size()-1);
        return _values[idx];
    }

private:
    std::vector<T> _values;
};


static titan::object*
random_built_in_variable(const std::string& name)
{
  RandomEntry<titan::obj_type> vt_entry({
    titan::obj_type::INT,
    titan::obj_type::FLOAT,
    });

  switch(vt_entry.get_value()) {
  case titan::obj_type::INT: return new titan::object_int(0);
  case titan::obj_type::FLOAT: return new titan::object_float(3.14159);
  default: return nullptr; // wont happen
  }

}

}
#endif
