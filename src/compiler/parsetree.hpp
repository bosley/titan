#ifndef COMPILER_PARSE_TREE_HPP
#define COMPILER_PARSE_TREE_HPP

#include "tokens.hpp"
#include <iostream>
#include <memory>
#include <utility>
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
  FLOAT,
  STRING,
  ARRAY,
  NIL,
  USER_DEFINED,
};

extern variable_types string_to_variable_type(const std::string &s);

struct variable {
  std::string name;
  variable_types type;
  std::string type_string;
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
  ARRAY_IDX
};

class expression {
public:
  expression() : type(node_type::ROOT) {}
  expression(node_type t) : type(t) {}
  expression(size_t line, size_t col, node_type t) : type(t) {}
  expression(node_type t, std::string val) : type(t), value(val) {}
  expression(size_t line, size_t col, node_type t, std::string val) : type(t), value(val), line(line), col(col) {}
  virtual ~expression() = default;

  node_type type;
  std::string value;
  size_t line;
  size_t col;
};
using expr_ptr = std::unique_ptr<expression>;

/*
 *  prefix / infix and other expression implementations are used to construct
 * expression trees this method will display the expression trees horizontally
 * via stdout
 */
extern void display_expr_tree(const std::string &prefix, expression *n,
                              bool is_left);

class prefix_expr : public expression {
public:
  prefix_expr(size_t line, size_t col, std::string op, expr_ptr right)
      : expression(line, col, node_type::PREFIX), op(op), right(std::move(right))
  {
  }
  std::string op;
  expr_ptr right;
};
using prefix_expr_ptr = std::unique_ptr<prefix_expr>;

class infix_expr : public expression {
public:
  infix_expr(size_t line, size_t col, std::string op, expr_ptr left, expr_ptr right)
      : expression(line, col, node_type::INFIX), op(op), left(std::move(left)),
        right(std::move(right))
  {
  }

  std::string op;
  expr_ptr left;
  expr_ptr right;
};
using infix_expr_ptr = std::unique_ptr<infix_expr>;

class array_literal_expr : public expression {
public:
  array_literal_expr(size_t line, size_t col) : expression(line, col, node_type::ARRAY) {}

  std::vector<expr_ptr> expressions;
};
using array_literal_expr_ptr = std::unique_ptr<array_literal_expr>;

class array_index_expr : public expression {
public:
  array_index_expr(size_t line, size_t col) : expression(line, col, node_type::ARRAY_IDX) {}
  array_index_expr(size_t line, size_t col, expr_ptr arr, expr_ptr idx)
      : expression(line, col, node_type::ARRAY_IDX), arr(std::move(arr)),
        index(std::move(idx))
  {
  }

  expr_ptr arr;
  expr_ptr index;
};
using array_index_expr_ptr = std::unique_ptr<array_index_expr>;

class function_call_expr : public expression {
public:
  function_call_expr(size_t line, size_t col) : expression(line, col, node_type::CALL) {}
  function_call_expr(size_t line, size_t col, expr_ptr fn)
      : expression(line, col, node_type::CALL), fn(std::move(fn))
  {
  }

  expr_ptr fn;
  std::vector<expr_ptr> params;
};
using function_call_expr_ptr = std::unique_ptr<function_call_expr>;

class visitor;

class element {
public:
  element() = delete;
  element(size_t line, size_t col) : line(line), col(col) {}
  virtual ~element() = default;
  virtual void visit(visitor &visitor) = 0;
  size_t line;
  size_t col;
};
using element_ptr = std::unique_ptr<element>;

class assignment_statement : public element {
public:
  assignment_statement(size_t line, size_t col, variable var, expr_ptr node)
      : element(line, col), var(var), expr(std::move(node))
  {
  }

  variable var;
  expr_ptr expr;

  virtual void visit(visitor &v) override;
};
using assignment_statement_ptr = std::unique_ptr<assignment_statement>;

class if_statement : public element {
public:
  class segment {
  public:
    segment(expr_ptr expr, std::vector<element_ptr> element_list)
        :

          expr(std::move(expr)), element_list(std::move(element_list))
    {
    }

    expr_ptr expr;
    std::vector<element_ptr> element_list;
  };
  if_statement(size_t line, size_t col) : element(line, col) {}

  std::vector<segment> segments;

  virtual void visit(visitor &v) override;
};
using if_statement_ptr = std::unique_ptr<if_statement>;

class expression_statement : public element {
public:
  expression_statement(size_t line, size_t col, expr_ptr node)
      : element(line, col), expr(std::move(node))
  {
  }

  expr_ptr expr;

  virtual void visit(visitor &v) override;
};
using expression_statement_ptr = std::unique_ptr<expression_statement>;

class while_statement : public element {
public:
  while_statement(size_t line, size_t col) : element(line, col), condition(nullptr) {}
  while_statement(size_t line, size_t col, expr_ptr c, std::vector<element_ptr> body)
      : element(line, col), condition(std::move(c)), body(std::move(body))
  {
  }

  expr_ptr condition;
  std::vector<element_ptr> body;

  virtual void visit(visitor &v) override;
};
using while_statement_ptr = std::unique_ptr<while_statement>;

class for_statement : public element {
public:
  for_statement(size_t line, size_t col) : element(line, col), condition(nullptr) {}
  for_statement(size_t line, size_t col, element_ptr assign, expr_ptr condition,
                expr_ptr modifier, std::vector<element_ptr> body)
      : element(line, col), assign(std::move(assign)),
        condition(std::move(condition)), modifier(std::move(modifier)),
        body(std::move(body))
  {
  }

  element_ptr assign;
  expr_ptr condition;
  expr_ptr modifier;
  std::vector<element_ptr> body;

  virtual void visit(visitor &v) override;
};
using for_statement_ptr = std::unique_ptr<for_statement>;

class return_statement : public element {
public:
  return_statement(size_t line, size_t col, expr_ptr node)
      : element(line, col), expr(std::move(node))
  {
  }
  expr_ptr expr;

  virtual void visit(visitor &v) override;
};
using return_statement_ptr = std::unique_ptr<return_statement>;

class toplevel {
public:
  enum class tl_type { IMPORT, FUNCTION };
  toplevel(tl_type t, size_t line, size_t col) : type(t), line(line), col(col)
  {
  }
  virtual ~toplevel() = default;
  tl_type type;
  size_t line;
  size_t col;
};
using toplevel_ptr = std::unique_ptr<toplevel>;

class import : public toplevel {
public:
  import(std::string target, size_t line, size_t col)
      : toplevel(toplevel::tl_type::IMPORT, line, col), target(target)
  {
  }
  import(size_t line, size_t col)
      : toplevel(toplevel::tl_type::IMPORT, line, col)
  {
  }
  std::string target;
};
using import_ptr = std::unique_ptr<import>;

class function : public toplevel {
public:
  function(size_t line, size_t col)
      : toplevel(toplevel::tl_type::FUNCTION, line, col)
  {
  }
  std::string name;
  std::string file_name;
  variable_types return_type;
  std::vector<variable> parameters;
  std::vector<element_ptr> element_list;
};
using function_ptr = std::unique_ptr<function>;

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
