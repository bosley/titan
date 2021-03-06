#ifndef TITAN_ENV_HPP
#define TITAN_ENV_HPP

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

  // Add an xfunc into the environment.
  // Will fail if the name is not unique
  bool add_xfunc(const std::string& name, xfunc *env_if);

  // Attempt to a variable from the environment for external use
  instructions::variable* get_variable(const std::string& name);

  // Create a new variable
  // Cleanup of given varible will be handled by internally
  bool new_variable(instructions::variable *var, bool global=true);

private:
  std::unordered_map<std::string, xfunc*> _external;
};



}


#endif
