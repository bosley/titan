#ifndef COMPILER_PARSE_TREE_HPP
#define COMPILER_PARSE_TREE_HPP

#include "tokens.hpp"
#include <iostream>
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
  RAW_NUMBER, // Literal number with no assigned type
  FLOAT,
  STRING,
  USER_DEFINED,
  OP,
  EXPR
};

static variable_types string_to_variable_type(const std::string &s)
{
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
  if (s == "float") {
    return variable_types::FLOAT;
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

enum class node_type {
  ROOT,
  ID,
  CALL,
  RAW_FLOAT,
  RAW_NUMBER,
  RAW_STRING,
  PREFIX,
  INFIX,
  ARRAY,
  ARRAY_IDX,
};

class expression {
public:
  expression() : type(node_type::ROOT) {}
  expression(node_type t) : type(t) {}
  expression(node_type t, std::string val) : type(t), value(val) {}

  node_type type;
  std::string value;
};

class prefix_expr : public expression {
public:
  prefix_expr(std::string op, expression *right)
      : expression(node_type::PREFIX), op(op), right(right)
  {
  }
  ~prefix_expr() { delete right; }
  std::string op;
  expression *right;
};

class infix_expr : public expression {
public:
  infix_expr(std::string op, expression *left, expression *right)
      : expression(node_type::INFIX), op(op), left(left), right(right)
  {
  }
  ~infix_expr()
  {
    delete left;
    delete right;
  }
  std::string op;
  expression *left;
  expression *right;
};

class array_literal_expr : public expression {
public:
  array_literal_expr() : expression(node_type::ARRAY) {}
  ~array_literal_expr()
  {
    for (auto &e : expressions) {
      delete e;
    }
  }
  std::vector<expression *> expressions;
};

class array_index_expr : public expression {
public:
  array_index_expr() : expression(node_type::ARRAY_IDX) {}
  array_index_expr(expression *arr, expression *idx)
      : expression(node_type::ARRAY_IDX), arr(arr), index(idx)
  {
  }
  ~array_index_expr()
  {
    delete index;
    delete arr;
  }
  expression *arr;
  expression *index;
};

class function_call_expr : public expression {
public:
  function_call_expr() : expression(node_type::CALL) {}
  function_call_expr(expression *fn) : expression(node_type::CALL), fn(fn) {}
  ~function_call_expr()
  {
    for (auto &e : params) {
      delete e;
    }
    delete fn;
  }
  expression *fn;
  std::vector<expression *> params;
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
  assignment(size_t line, variable var, expression *node)
      : element(line), var(var), expr(node)
  {
  }
  variable var;
  expression *expr;

  virtual void visit(visitor &v) override;
};

class expression_statement : public element {
public:
  expression_statement(size_t line, expression *node)
      : element(line), expr(node)
  {
  }
  expression *expr;

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
      : toplevel(toplevel::tl_type::IMPORT), target(target)
  {
  }
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
  virtual void accept(expression_statement &stmt) = 0;
};

static void display_expr_tree(const std::string &prefix, expression *n,
                              bool is_left)
{
  if (!n) {
    return;
  }
  std::cout << prefix;
  std::cout << (is_left ? "├──" : "└──");

  if (n->type == node_type::CALL) {
    auto i = reinterpret_cast<function_call_expr *>(n);
    if (i->fn) {
      std::cout << " call<" << i->fn->value << ">" << std::endl;
    }
    else {
      std::cout << " call " << std::endl;
    }
  }
  else if (n->type == node_type::ARRAY_IDX) {
    auto i = reinterpret_cast<array_index_expr *>(n);
    if (i->arr && i->index) {
      std::cout << " " << i->arr->value << "[" << i->index->value << "]"
                << std::endl;
    }
    else {
      std::cout << " array[] " << std::endl;
    }
  }
  else if (n->type == node_type::INFIX) {
    auto i = reinterpret_cast<infix_expr *>(n);
    std::cout << " " << i->op << std::endl;
    display_expr_tree(prefix + (is_left ? "│   " : "    "), i->left, true);
    display_expr_tree(prefix + (is_left ? "│   " : "    "), i->right, false);
  }
  else if (n->type == node_type::PREFIX) {
    auto i = reinterpret_cast<prefix_expr *>(n);
    std::cout << " " << i->op << std::endl;
    display_expr_tree(prefix + (is_left ? "│   " : "    "), i->right, false);
  }
  else {
    std::cout << " " << n->value << std::endl;
  }
}

} // namespace parse_tree

} // namespace compiler

#endif
