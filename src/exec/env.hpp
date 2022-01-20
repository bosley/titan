#ifndef TITAN_ENV_HPP
#define TITAN_ENV_HPP

#include "space.hpp"
#include "memory.hpp"
#include "lang/instructions.hpp"

#include <unordered_map>
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
    // Name the full scoping and name of the item ( std::io::print )
    // Parameters as a vector of 'instruction::variable' 
    // Reuturn type as an 'instruction::variable'
    
    // Execute the functionality of this xfunc using the given parameters
    // The return_type variable will be used as the return data of the call
    virtual void execute() = 0;
  };

  env();

  // Add an xfunc into the environment.
  // Will fail if the name is not unique
  bool add_xfunc(const std::string& name, xfunc *env_if);

  instructions::variable* get_variable(const std::string& space, const std::string& name);
  bool new_variable(const std::string& space, instructions::variable* var);

private:
  
  std::unique_ptr<memory>  _memory;

  // Callable functions
  std::unordered_map<std::string, xfunc*> _external;
};



}


#endif
