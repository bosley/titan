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

extern variable_types string_to_variable_type(const std::string &s);

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
  REASSIGN
};

class expression {
public:
  expression() : type(node_type::ROOT) {}
  expression(node_type t) : type(t) {}
  expression(node_type t, std::string val) : type(t), value(val) {}

  node_type type;
  std::string value;
};
using expr_ptr = std::shared_ptr<expression>;

/*
 *  prefix / infix and other expression implementations are used to construct expression trees
 *  this method will display the expression trees horizontally via stdout
 */
extern void display_expr_tree(const std::string &prefix, expr_ptr n, bool is_left);

class prefix_expr : public expression {
public:
  prefix_expr(std::string op, expr_ptr right)
      : expression(node_type::PREFIX), op(op), right(right)
  {
  }
  std::string op;
  expr_ptr right;
};
using prefix_expr_ptr = std::shared_ptr<prefix_expr>;

class infix_expr : public expression {
public:
  infix_expr(std::string op, expr_ptr left, expr_ptr right)
      : expression(node_type::INFIX), op(op), left(left), right(right)
  {
  }

  std::string op;
  expr_ptr left;
  expr_ptr right;
};
using infix_expr_ptr = std::shared_ptr<infix_expr>;

class array_literal_expr : public expression {
public:
  array_literal_expr() : expression(node_type::ARRAY) {}
  
  std::vector<expr_ptr > expressions;
};
using array_literal_expr_ptr = std::shared_ptr<array_literal_expr>;

class array_index_expr : public expression {
public:
  array_index_expr() : expression(node_type::ARRAY_IDX) {}
  array_index_expr(expr_ptr arr, expr_ptr idx)
      : expression(node_type::ARRAY_IDX), arr(arr), index(idx)
  {
  }

  expr_ptr arr;
  expr_ptr index;
};
using array_index_expr_ptr = std::shared_ptr<array_index_expr>;

class function_call_expr: public expression {
public:
  function_call_expr() : expression(node_type::CALL) {}
  function_call_expr(expr_ptr fn) : expression(node_type::CALL), fn(fn) {}

  expr_ptr fn;
  std::vector<expr_ptr > params;
};
using function_call_expr_ptr = std::shared_ptr<function_call_expr>;

class visitor;

class element {
public:
  element() = delete;
  element(size_t line_number) : line_number(line_number) {}
  virtual ~element() = default;
  virtual void visit(visitor &visitor) = 0;
  size_t line_number;
};
using element_ptr = std::shared_ptr<element>;

class assignment_statement : public element {
public:
  assignment_statement(size_t line, variable var, expr_ptr node)
      : element(line), var(var), expr(node)
  {
  }

  variable var;
  expr_ptr expr;

  virtual void visit(visitor &v) override;
};
using assignment_statement_ptr = std::shared_ptr<assignment_statement>;

class if_statement : public element {
public:
  struct segment {
    expr_ptr expr;
    std::vector<element_ptr> element_list;
  };
  if_statement(size_t line) : element(line) {}

  if_statement(size_t line, expr_ptr expr) : element(line)
  {
    segments.push_back({expr, {}});
  }

  std::vector<segment> segments;

  virtual void visit(visitor &v) override;
};
using if_statement_ptr = std::shared_ptr<if_statement>;

class expression_statement : public element {
public:
  expression_statement(size_t line, expr_ptr node)
      : element(line), expr(node)
  {
  }

  expr_ptr expr;

  virtual void visit(visitor &v) override;
};
using expression_statement_ptr = std::shared_ptr<expression_statement>;

class while_statement : public element {
public:
  while_statement(size_t line) : element(line), condition(nullptr) {}
  while_statement(size_t line, expr_ptr c, std::vector<element_ptr> body)
      : element(line), condition(c), body(body)
  {
  }

  expr_ptr condition;
  std::vector<element_ptr> body;

  virtual void visit(visitor &v) override;
};
using while_statement_ptr = std::shared_ptr<while_statement>;

class for_statement : public element {
public:
  for_statement(size_t line) : element(line), condition(nullptr) {}
  for_statement(size_t line, element_ptr assign, expr_ptr condition,
                expr_ptr modifier, std::vector<element_ptr> body)
      : element(line), assign(assign), condition(condition), modifier(modifier),
        body(body)
  {
  }

  element_ptr assign;
  expr_ptr condition;
  expr_ptr modifier;
  std::vector<element_ptr> body;

  virtual void visit(visitor &v) override;
};
using for_statement_ptr = std::shared_ptr<for_statement>;

class return_statement : public element {
public:
  return_statement(size_t line, expr_ptr node) : element(line), expr(node) {}
  expr_ptr expr;

  virtual void visit(visitor &v) override;
};
using return_statement_ptr = std::shared_ptr<return_statement>;

class toplevel {
public:
  enum class tl_type { IMPORT, FUNCTION };
  toplevel(tl_type t) : type(t) {}
  virtual ~toplevel() = default;
  tl_type type;
};
using toplevel_ptr = std::shared_ptr<toplevel>;

class import : public toplevel {
public:
  import(std::string target)
      : toplevel(toplevel::tl_type::IMPORT), target(target)
  {
  }
  import() : toplevel(toplevel::tl_type::IMPORT) {}
  std::string target;
};
using import_ptr = std::shared_ptr<import>;

class function : public toplevel {
public:
  function() : toplevel(toplevel::tl_type::FUNCTION) {}
  std::string name;
  variable_types return_type;
  std::vector<variable> parameters;
  std::vector<element_ptr> element_list;
};
using function_ptr = std::shared_ptr<function>;

class visitor {
public:
  virtual void accept(assignment_statement &stmt) = 0;
  virtual void accept(expression_statement &stmt) = 0;
  virtual void accept(if_statement &stmt) = 0;
  virtual void accept(while_statement &stmt) = 0;
  virtual void accept(for_statement &stmt) = 0;
  virtual void accept(return_statement &stmt) = 0;
};

} // namespace parse_tree

} // namespace compiler

#endif
