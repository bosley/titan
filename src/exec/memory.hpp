#ifndef TITAN_MEMORY_HPP
#define TITAN_MEMORY_HPP

#include "space.hpp"
#include "types/types.hpp"

#include <memory>
#include <unordered_map>

namespace titan
{

//  Assoicates variable storage spaces to mapped scope names
//  so we can have a many-to-one storage list
class memory
{
public:
  memory();
  ~memory();

  //  Create a memory segment with a unique name
  //  to encapsulate a file or REPL environment
  bool new_space(const std::string& name);

  //  Add a name translation (like a DBA) so a space can be referred to 
  //  by another name (for relative accessing)
  bool associate_space_with_name(const std::string& space, const std::string& name);
  
  //  Create a new variable
  //  space - Storage unit for data
  //  name  - The name of the variable
  //  var   - The variable (moved and owned by memory after called)
  //  Returns true iff the space exists or can be translated to a space
  bool new_variable(const std::string& space, const std::string& name, object *var);

  //  Attempt to get a variable from a space 
  //  space - Storage unit for the data
  //  name  - The name of the variable
  //  Returns variable pointer or nullptr 
  object* get_variable(const std::string& space, const std::string& name);

  //  Attempt to delete an existing variable
  //  space - Storage unit for the data
  //  name  - The name of the variable
  //  Returns true iff the space and variable exist AND it was able to be deleted
  bool delete_variable(const std::string& space, const std::string& name);

private:

  //  Every loaded file or REPL env will be stored in 
  //  a 'space' owned by this memory object
  std::unordered_map<std::string, std::unique_ptr<space> > _spaces;
  std::unordered_map<std::string, std::string > _space_translation;
};

}

#endif
