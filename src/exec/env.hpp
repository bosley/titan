#ifndef TITAN_ENV_HPP
#define TITAN_ENV_HPP

#include "space.hpp"
#include "memory.hpp"
#include "types/types.hpp"

#include <unordered_map>
#include <vector>
#include <optional>

namespace titan
{

//  Execution environment for titan
//
class env {
public:

  // An xfunc to extend the functionality of titan (external function)
  class xfunc {
  public:
    virtual ~xfunc() = default;
    virtual void execute() = 0;

    std::vector<object*> parameters;
    object * result;
  };

  env();

  // Add an xfunc into the environment.
  // Will fail if the name is not unique
  bool add_xfunc(const std::string& name, xfunc *env_if);

  bool new_space(const std::string& name);

  bool associate_space(const std::string& space, const std::string& name);

  object* get_variable(const std::string& space, const std::string& name);
  bool new_variable(const std::string& space, const std::string& name, object* var);

  xfunc* get_external_function(const std::string& name);

private:
  
  std::unique_ptr<memory>  _memory;

  // Callable functions
  std::unordered_map<std::string, xfunc*> _external;
};



}


#endif
