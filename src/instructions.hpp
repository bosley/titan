#ifndef TITAN_INSTRUCTIONS_HPP
#define TITAN_INSTRUCTIONS_HPP

#include "tokens.hpp"
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace titan {
namespace instructions {

enum class variable_types {
  U8 = 0,
  U16,
  U32,
  U64,
  I8,
  I16,
  I32,
  I64,
  FLOAT = 20,
  STRING,
  ARRAY,
  USER_DEFINED,
  NIL,
};

extern variable_types string_to_variable_type(const std::string &s);

struct vtd {
  variable_types type;
  uint64_t depth;
  std::vector<uint64_t> segments;
};

struct variable {
  std::string name;
  std::string type_string;
  vtd type_depth;
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
  expression(size_t line, size_t col, node_type t)
      : line(line), col(col), type(t)
  {
  }
  expression(node_type t, std::string val) : type(t), value(val) {}
  expression(size_t line, size_t col, node_type t, std::string val)
      : line(line), col(col), type(t), value(val)
  {
  }
  virtual ~expression() = default;

  size_t line;
  size_t col;
  node_type type;
  std::string value;
};
using expr_ptr = std::unique_ptr<expression>;

/*
 *  prefix / infix and other expression implementations are used to construct
 * expression trees this method will display the expression trees horizontally
 * via stdout
 */
extern void display_expr_tree(const std::string &prefix, expression *n,
                              bool is_left);

class raw_int_expr : public expression {
public:
  raw_int_expr(size_t line, size_t col, std::string val)
      : expression(line, col, node_type::RAW_NUMBER, val),
        as(variable_types::I64), with_val(0)
  {
  }
  raw_int_expr(size_t line, size_t col, std::string val, variable_types as,
               long long ival)
      : expression(line, col, node_type::RAW_NUMBER, val), as(as),
        with_val(ival)
  {
  }
  variable_types as;
  long long with_val;
};
using raw_int_expr_ptr = std::unique_ptr<raw_int_expr>;

class prefix_expr : public expression {
public:
  prefix_expr(size_t line, size_t col, std::string op, expr_ptr right)
      : expression(line, col, node_type::PREFIX), op(op),
        right(std::move(right))
  {
  }
  std::string op;
  Token tok_op;
  expr_ptr right;
};
using prefix_expr_ptr = std::unique_ptr<prefix_expr>;

class infix_expr : public expression {
public:
  infix_expr(size_t line, size_t col, std::string op, expr_ptr left,
             expr_ptr right)
      : expression(line, col, node_type::INFIX), op(op), left(std::move(left)),
        right(std::move(right))
  {
  }

  std::string op;
  Token tok_op;

  expr_ptr left;
  expr_ptr right;
};
using infix_expr_ptr = std::unique_ptr<infix_expr>;

class array_literal_expr : public expression {
public:
  array_literal_expr(size_t line, size_t col)
      : expression(line, col, node_type::ARRAY)
  {
  }

  std::vector<expr_ptr> expressions;
};
using array_literal_expr_ptr = std::unique_ptr<array_literal_expr>;

class array_index_expr : public expression {
public:
  array_index_expr(size_t line, size_t col)
      : expression(line, col, node_type::ARRAY_IDX)
  {
  }
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
  function_call_expr(size_t line, size_t col)
      : expression(line, col, node_type::CALL)
  {
  }
  function_call_expr(size_t line, size_t col, expr_ptr fn)
      : expression(line, col, node_type::CALL), fn(std::move(fn))
  {
  }
  function_call_expr(size_t line, size_t col, expr_ptr fn,
                     std::vector<expr_ptr> p)
      : expression(line, col, node_type::CALL), fn(std::move(fn)),
        params(std::move(p))
  {
  }

  expr_ptr fn;
  std::vector<expr_ptr> params;
};
using function_call_expr_ptr = std::unique_ptr<function_call_expr>;

class visitor;

class instruction {
public:
  instruction() = delete;
  instruction(size_t line, size_t col) : line(line), col(col) {}
  virtual ~instruction() = default;
  virtual void visit(visitor &visitor) = 0;
  size_t line;
  size_t col;
};
using instruction_ptr = std::unique_ptr<instruction>;

class assignment_statement : public instruction {
public:
  assignment_statement(size_t line, size_t col, variable var, expr_ptr node)
      : instruction(line, col), var(var), expr(std::move(node))
  {
  }

  variable var;
  expr_ptr expr;

  virtual void visit(visitor &v) override;
};
using assignment_statement_ptr = std::unique_ptr<assignment_statement>;

class if_statement : public instruction {
public:
  class segment {
  public:
    segment(expr_ptr expr, std::vector<instruction_ptr> instruction_list)
        :

          expr(std::move(expr)), instruction_list(std::move(instruction_list))
    {
    }

    expr_ptr expr;
    std::vector<instruction_ptr> instruction_list;
  };
  if_statement(size_t line, size_t col) : instruction(line, col) {}

  std::vector<segment> segments;

  virtual void visit(visitor &v) override;
};
using if_statement_ptr = std::unique_ptr<if_statement>;

class expression_statement : public instruction {
public:
  expression_statement(size_t line, size_t col, expr_ptr node)
      : instruction(line, col), expr(std::move(node))
  {
  }

  expr_ptr expr;

  virtual void visit(visitor &v) override;
};
using expression_statement_ptr = std::unique_ptr<expression_statement>;

class while_statement : public instruction {
public:
  while_statement(size_t line, size_t col)
      : instruction(line, col), condition(nullptr)
  {
  }
  while_statement(size_t line, size_t col, expr_ptr c,
                  std::vector<instruction_ptr> body)
      : instruction(line, col), condition(std::move(c)), body(std::move(body))
  {
  }

  expr_ptr condition;
  std::vector<instruction_ptr> body;

  virtual void visit(visitor &v) override;
};
using while_statement_ptr = std::unique_ptr<while_statement>;

class for_statement : public instruction {
public:
  for_statement(size_t line, size_t col)
      : instruction(line, col), condition(nullptr)
  {
  }
  for_statement(size_t line, size_t col, instruction_ptr assign, expr_ptr condition,
                expr_ptr modifier, std::vector<instruction_ptr> body)
      : instruction(line, col), assign(std::move(assign)),
        condition(std::move(condition)), modifier(std::move(modifier)),
        body(std::move(body))
  {
  }

  instruction_ptr assign;
  expr_ptr condition;
  expr_ptr modifier;
  std::vector<instruction_ptr> body;

  virtual void visit(visitor &v) override;
};
using for_statement_ptr = std::unique_ptr<for_statement>;

class return_statement : public instruction {
public:
  return_statement(size_t line, size_t col, expr_ptr node)
      : instruction(line, col), expr(std::move(node))
  {
  }
  expr_ptr expr;

  virtual void visit(visitor &v) override;
};
using return_statement_ptr = std::unique_ptr<return_statement>;

class import : public instruction {
public:
  import(std::string target, size_t line, size_t col)
      : instruction(line, col), target(target)
  {
  }
  import(size_t line, size_t col)
      : instruction(line, col)
  {
  }
  std::string target;
  virtual void visit(visitor &v) override;
};
using import_ptr = std::unique_ptr<import>;

class function : public instruction {
public:
  function(size_t line, size_t col)
      : instruction(line, col)
  {
  }
  std::string name;
  std::string file_name;
  vtd return_data;
  std::vector<variable> parameters;
  std::vector<instruction_ptr> instruction_list;
  virtual void visit(visitor &v) override;
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
  virtual void accept(import &stmt) = 0;
  virtual void accept(function &stmt) = 0;
};

} // namespace parse_tree

} // namespace compiler

#endif
