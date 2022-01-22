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
    titan::obj_type::U8,
    titan::obj_type::U16,
    titan::obj_type::U32,
    titan::obj_type::U64,
    titan::obj_type::I8,
    titan::obj_type::I16,
    titan::obj_type::I32,
    titan::obj_type::I64,
    titan::obj_type::FLOAT,
    });

  switch(vt_entry.get_value()) {
  case titan::obj_type::U8: return new titan::object_u8(0);
  case titan::obj_type::U16: return new titan::object_u16(0);
  case titan::obj_type::U32: return new titan::object_u32(0);
  case titan::obj_type::U64: return new titan::object_u64(0);
  case titan::obj_type::I8: return new titan::object_i8(0);
  case titan::obj_type::I16: return new titan::object_i16(0);
  case titan::obj_type::I32: return new titan::object_i32(0);
  case titan::obj_type::I64: return new titan::object_i64(0);
  case titan::obj_type::FLOAT: return new titan::object_float(3.14159);
  default: return nullptr; // wont happen
  }

}

}
#endif
