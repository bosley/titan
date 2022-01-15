#include "env.hpp"

namespace titan
{

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

instructions::variable* env::get_variable(const std::string& name)
{
  return nullptr;
}

bool env::new_variable(instructions::variable *var, bool global)
{
  return false;
}


}
