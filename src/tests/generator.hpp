#ifndef GENERATOR_TESTS_HPP
#define GENERATOR_TESTS_HPP

#include "lang/instructions.hpp"

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


static titan::instructions::variable*
random_built_in_variable(const std::string& name)
{
   
  GenerateRandom<uint64_t> g;

  uint64_t depth = g.get_range(0, 10);
  uint64_t num_segments = g.get_range(0, 5);

  std::vector<uint64_t> segments;
  for(auto i = 0; i < num_segments; i++)
  {
    segments.push_back(g.get_range(1, 5));
  }

  RandomEntry<titan::instructions::variable_types> vt_entry({
    titan::instructions::variable_types::U8,
    titan::instructions::variable_types::U16,
    titan::instructions::variable_types::U32,
    titan::instructions::variable_types::U64,
    titan::instructions::variable_types::I8,
    titan::instructions::variable_types::I16,
    titan::instructions::variable_types::I32,
    titan::instructions::variable_types::I64,
    titan::instructions::variable_types::FLOAT,
    });

  return new titan::instructions::built_in_variable(
      name, vt_entry.get_value(), depth, segments);
}

}
#endif
