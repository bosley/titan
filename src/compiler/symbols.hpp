#ifndef SYMBOLS_HPP
#define SYMBOLS_HPP

#include "parsetree.hpp"

#include <string>
#include <variant>
#include <vector>
#include <tuple>
#include <optional>

/* 
 
   Idea time.

   Functions need to be searched, but they don't exist within
   any scope other than 'global' at least for now.

   Scopes within functions need to be able to find anything in
   itsself or anything directly above it all the way to global   
 
 */

namespace compiler {

namespace symbol {

using variant_data =
    std::variant<parse_tree::assignment_statement *, parse_tree::function *>;

class table {

public:

  table();

  void set_scope_to_global();
  void add_scope(const std::string &name);
  void add_scope_and_enter(const std::string &name);
  void pop_scope();

  // Begin operating within a top level scope with name
  bool activate_top_level_scope(const std::string &name);

  // Add a function to the current scope's symbol table
  bool add_symbol(const std::string &name, parse_tree::function *);

  // Add a variable to the current scope's symbol table
  bool add_symbol(const std::string &name, parse_tree::assignment_statement *);

  //  Check to see if a symbol exists within reach
  //  Marking current_only will limit search to current scope
  bool exists(const std::string &v, bool current_only = false);

  //  Attempt to find a symbol
  //  Marking current_only will limit search to current scope
  std::optional<variant_data> lookup(const std::string &v,
                                        bool current_only = false);

private:
  
  // Table entry for a given scope
  struct table_entry {
    std::string name;
    variant_data data;
  };

  // Scope that contains data, and potentially contains sub scopes
  class scope {
  public:
    scope(const std::string& name) : name(name), prev_scope(nullptr) {}
    ~scope()
    {
      for (auto &s : sub_scopes) {
        delete s;
      }
    }
    std::string name;
    scope *prev_scope;                // So search can happen from scope
    std::vector<table_entry> entries; // Data in scope (usually assignments)
    std::vector<scope *> sub_scopes;  // Inner scopes from current 
  };

  scope _global_scope; // Program global scope (functions)
  scope *_curr_scope;  // Scope currently being populated

  bool scope_contains_item(scope *s, const std::string &v);
};  
}
}

#endif
