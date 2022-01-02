#ifndef SYMBOLS_HPP
#define SYMBOLS_HPP

#include "parsetree.hpp"

#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace compiler {

namespace symbol {

enum class variant_type { ASSIGNMENT, FUNCTION };

struct variant_data {
  variant_type type;
  union {
    parse_tree::assignment_statement *assignment;
    parse_tree::function *function;
  };
};

class table {

public:
  table();

  // Set to top level scope
  void set_scope_to_global();

  // Add a subscope with name, but do not enter
  void add_scope(const std::string &name);

  // Add a subscope with name, and enter
  void add_scope_and_enter(const std::string &name);

  // Leave scope for parent scope (if no parent scope will stop at global)
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
    scope(const std::string &name) : name(name), prev_scope(nullptr) {}
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
} // namespace symbol
} // namespace compiler

#endif
