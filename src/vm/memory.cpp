#include "memory.hpp"
#include "lang/instructions.hpp"

#include <memory>

namespace titan
{

memory::memory()
{

}

bool memory::new_space(const std::string &name)
{
  if(_spaces.find(name) != _spaces.end()) {
    return false;
  }
  _spaces[name] = std::unique_ptr<space>(new space());
  return true;
}

bool memory::associate_space_with_name(const std::string& space, const std::string& name)
{
  return false;
}

bool memory::new_variable(const std::string& space, const std::string& name, instructions::variable *var)
{
  return false;
}

instructions::variable* memory::get_variable(const std::string& space, const std::string& name)
{
  return nullptr;
}

bool memory::delete_variable(const std::string& space, const std::string& name)
{
  return false;
}

}
