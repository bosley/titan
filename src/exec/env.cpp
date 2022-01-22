#include "env.hpp"

namespace titan
{

env::env()
{
  _memory = std::unique_ptr<memory>(new memory());
}

bool env::add_xfunc(const std::string& name, xfunc *env_if)
{
  if(!env_if) {
    return false;
  }

  if(_external.find(name) != _external.end()) {
    return false;
  }

  _external[name] = env_if;
  return true;
}

bool env::new_space(const std::string& space)
{
  return _memory->new_space(space);
}

bool env::associate_space(const std::string& space, const std::string& name)
{
  return _memory->associate_space_with_name(space, name);
}

object* env::get_variable(const std::string& space, const std::string& name)
{
  return _memory->get_variable(space, name);
}

bool env::new_variable(const std::string& space, const std::string& name, object* var)
{
  return _memory->new_variable(space, name, var);
}

}
