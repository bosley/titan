#ifndef COMPILER_PARSE_TREE_HPP
#define COMPILER_PARSE_TREE_HPP

#include "tokens.hpp"
#include <vector>

namespace compiler {

namespace parse_tree {

enum class variable_types {
  U8 = 0,
  U16,
  U32,
  U64,
  I8,
  I16,
  I32,
  I64,
  DOUBLE,
  STRING,
  USER_DEFINED
};

static variable_types string_to_variable_type(const std::string &s) {
  if (s == "u8") {
    return variable_types::U8;
  }
  if (s == "u16") {
    return variable_types::U16;
  }
  if (s == "u32") {
    return variable_types::U32;
  }
  if (s == "u64") {
    return variable_types::U64;
  }
  if (s == "i8") {
    return variable_types::I8;
  }
  if (s == "i16") {
    return variable_types::I16;
  }
  if (s == "i32") {
    return variable_types::I32;
  }
  if (s == "i64") {
    return variable_types::I64;
  }
  if (s == "double") {
    return variable_types::DOUBLE;
  }
  if (s == "string") {
    return variable_types::STRING;
  }
  return variable_types::USER_DEFINED;
}

struct variable {
  std::string name;
  variable_types type;
  uint64_t depth; // 0 For single variable, >0 for allocation space,
                  // uint64_t::max() for unknown size
};

enum class node_type { ROOT, ADD, SUB, DIV, MUL };

enum class node_value_type { NIL, INTEGER, FLOAT, STRING };

class expr_node {
public:
  expr_node()
      : type(node_type::ROOT), val_type(node_value_type::NIL), left(nullptr),
        right(nullptr) {}
  expr_node(node_type t, node_value_type nvt)
      : type(t), val_type(nvt), left(nullptr), right(nullptr) {}
  expr_node(node_type t, node_value_type nvt, expr_node *left, expr_node *right)
      : type(t), val_type(nvt), left(left), right(right) {}

  node_type type;
  node_value_type val_type;
  std::string value;

  expr_node *left;
  expr_node *right;
};

class visitor;

class element {
public:
  element() = delete;
  element(size_t line_number) : line_number(line_number) {}
  virtual ~element() = default;
  virtual void visit(visitor &visitor) = 0;
  size_t line_number;
};

class assignment : public element {
public:
  assignment(size_t line, variable var, expr_node *node)
      : element(line), var(var), expr(node) {}
  variable var;
  expr_node *expr;

  virtual void visit(visitor &v) override;
};

class expr_statement : public element {
public:
  expr_statement(size_t line, expr_node *node) : element(line), expr(node) {}
  expr_node *expr;

  virtual void visit(visitor &v) override;
};

class toplevel {
public:
  enum class tl_type { IMPORT, FUNCTION };
  toplevel(tl_type t) : type(t) {}
  virtual ~toplevel() = default;
  tl_type type;
};

class import_stmt : public toplevel {
public:
  import_stmt(std::string target)
      : toplevel(toplevel::tl_type::IMPORT), target(target) {}
  import_stmt() : toplevel(toplevel::tl_type::IMPORT) {}
  std::string target;
};

class function : public toplevel {
public:
  function() : toplevel(toplevel::tl_type::FUNCTION) {}
  std::string name;
  variable_types return_type;
  std::vector<variable> parameters;
  std::vector<element *> element_list;
};

class visitor {
public:
  virtual void accept(assignment &stmt) = 0;
  virtual void accept(expr_statement &stmt) = 0;
};

} // namespace parse_tree

} // namespace compiler

#endif