#include "memory.hpp"
#include "lang/instructions.hpp"

#include <memory>

namespace titan
{

memory::memory(){}

memory::~memory()
{
  //_spaces.clear();
  //_space_translation.clear();
}

bool memory::new_space(const std::string &name)
{
  if(_spaces.find(name) != _spaces.end()) {
    return false;
  }
  _spaces[name] = std::unique_ptr<space>(new space());
  return associate_space_with_name(name, name);
}

bool memory::associate_space_with_name(const std::string& space, const std::string& name)
{
  if(_spaces.find(space) == _spaces.end()) {
    return false;
  }
  _space_translation[name] = space;
  return true;
}

bool memory::new_variable(const std::string& space, instructions::variable *var)
{
  if(_space_translation.find(space) == _space_translation.end()) {
    return false;
  }
  auto target_space = _space_translation[space];
  return _spaces[target_space]->new_var(var);
}

instructions::variable* memory::get_variable(const std::string& space, const std::string& name)
{
  if(_space_translation.find(space) == _space_translation.end()) {
    return nullptr;
  }
  auto target_space = _space_translation[space];
  return _spaces[target_space]->get_variable(name);
}

bool memory::delete_variable(const std::string& space, const std::string& name)
{
  if(_space_translation.find(space) == _space_translation.end()) {
    return false;
  }
  auto target_space = _space_translation[space];
  return _spaces[target_space]->delete_var(name);
}

}
