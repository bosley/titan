#ifndef TITAN_SPACE_HPP
#define TITAN_SPACE_HPP

#include "types/types.hpp"

#include <unordered_map>
#include <stack>

namespace titan
{

class space
{
public:
  space();
  ~space();
  
  // Scopes that can only access global, and themselves (functions, etc)
  void push_top_level_scope();
  void pop_scope();

  // Drill into a top level scope (loop -> if -> etc)
  void sub_scope();
  void leave_scope();

  // Attempt to get a variable from the space
  object *get_variable(const std::string& name);

  // Attempt to delete a variable
  bool delete_var(const std::string& name);

  // Attempt to create a new variable
  bool new_var(const std::string& name, object *var);

private:
  struct scope
  {
    scope  *parent;
    scope *sub_scope;
    std::unordered_map<std::string, object_ptr> members;
  };

  uint64_t _scope_depth;
  scope _global_scope;
  scope *_operating_scope;

  // Individual functions and scopes that can only access themselves
  // and global_scope
  std::stack<scope> _top_level_scopes;
};

}


#endif
