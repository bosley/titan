/*
   This file defines 5 seperate classifications of types

   1) Meta types

      variable, built_in_variable, user_defined_variable ...

   2) Discriminators

      variable_types, variable_classifications, node_types ...

   3) Expressions

      expression_ptr, array_literal_expr, prefix_expr ...

   4) Instructions

      assignmet_instruction, for_instruction, while_instruction ...

   5) Receiver interface(s)

      Interfaces used by extgernal objects to 'receive' or be 'visited-by'
      groups of objects defined here

   All of these things are defined together to ease the process of development.
*/

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
  UNDEF,
};

enum class variable_classification { UNDEF, BUILT_IN, USER_DEFINED };

extern variable_types string_to_variable_type(const std::string &s);

class variable {
public:
  variable() : name(""), classification(variable_classification::UNDEF) {}
  variable(const std::string name, variable_classification vc)
      : name(name), classification(vc)
  {
  }
  virtual ~variable() = default;

  std::string name;
  variable_classification classification;
};
using variable_ptr = std::unique_ptr<variable>;

class built_in_variable : public variable {
public:
  built_in_variable(const std::string name)
      : variable(name, variable_classification::BUILT_IN)
  {
  }

  built_in_variable(const std::string name, variable_types type, uint64_t depth,
                    std::vector<uint64_t> seg)
      : variable(name, variable_classification::BUILT_IN), type(type),
        depth(depth), segments(seg)
  {
  }

  variable_types type;
  uint64_t depth;
  std::vector<uint64_t> segments;
};
using built_in_variable_ptr = std::unique_ptr<built_in_variable>;

class user_defined_variable : public variable {
public:
  user_defined_variable(const std::string name)
      : variable(name, variable_classification::USER_DEFINED)
  {
  }

  std::string type; // Name of the user type so it can be mapped
                    // to a definition later
};
using user_defined_variable_ptr = std::unique_ptr<user_defined_variable>;

class user_struct {
public:

  user_struct(){}
  ~user_struct() {
    for(auto &m : members) {
      delete m;
    }
  }

  std::string name;
  enum class def_scope { PUBLIC, PRIVATE };
  struct member_variable {
    variable_ptr var;
    def_scope scope;
  };
  std::vector<member_variable*> members;
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

class ins_receiver;

class instruction {
public:
  instruction() = delete;
  instruction(size_t line, size_t col) : line(line), col(col) {}
  virtual ~instruction() = default;
  virtual void visit(ins_receiver &ins_receiver) = 0;
  size_t line;
  size_t col;
};
using instruction_ptr = std::unique_ptr<instruction>;

class define_user_struct : public instruction {
public:
  define_user_struct(size_t line, size_t col, user_struct var)
      : instruction(line, col), var(var)
  {
  }

  user_struct var;

  virtual void visit(ins_receiver &v) override;
};
using define_user_struct_ptr = std::unique_ptr<define_user_struct>;

class assignment_instruction : public instruction {
public:
  assignment_instruction(size_t line, size_t col, variable_ptr var, expr_ptr node)
      : instruction(line, col), var(std::move(var)), expr(std::move(node))
  {
  }

  variable_ptr var;
  expr_ptr expr;

  virtual void visit(ins_receiver &v) override;
};
using assignment_instruction_ptr = std::unique_ptr<assignment_instruction>;

class if_instruction : public instruction {
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
  if_instruction(size_t line, size_t col) : instruction(line, col) {}

  std::vector<segment> segments;

  virtual void visit(ins_receiver &v) override;
};
using if_instruction_ptr = std::unique_ptr<if_instruction>;

class expression_instruction : public instruction {
public:
  expression_instruction(size_t line, size_t col, expr_ptr node)
      : instruction(line, col), expr(std::move(node))
  {
  }

  expr_ptr expr;

  virtual void visit(ins_receiver &v) override;
};
using expression_instruction_ptr = std::unique_ptr<expression_instruction>;

class while_instruction : public instruction {
public:
  while_instruction(size_t line, size_t col)
      : instruction(line, col), condition(nullptr)
  {
  }
  while_instruction(size_t line, size_t col, expr_ptr c,
                    std::vector<instruction_ptr> body)
      : instruction(line, col), condition(std::move(c)), body(std::move(body))
  {
  }

  expr_ptr condition;
  std::vector<instruction_ptr> body;

  virtual void visit(ins_receiver &v) override;
};
using while_instruction_ptr = std::unique_ptr<while_instruction>;

class for_instruction : public instruction {
public:
  for_instruction(size_t line, size_t col)
      : instruction(line, col), condition(nullptr)
  {
  }
  for_instruction(size_t line, size_t col, instruction_ptr assign,
                  expr_ptr condition, expr_ptr modifier,
                  std::vector<instruction_ptr> body)
      : instruction(line, col), assign(std::move(assign)),
        condition(std::move(condition)), modifier(std::move(modifier)),
        body(std::move(body))
  {
  }

  instruction_ptr assign;
  expr_ptr condition;
  expr_ptr modifier;
  std::vector<instruction_ptr> body;

  virtual void visit(ins_receiver &v) override;
};
using for_instruction_ptr = std::unique_ptr<for_instruction>;

class return_instruction : public instruction {
public:
  return_instruction(size_t line, size_t col, expr_ptr node)
      : instruction(line, col), expr(std::move(node))
  {
  }
  expr_ptr expr;

  virtual void visit(ins_receiver &v) override;
};
using return_instruction_ptr = std::unique_ptr<return_instruction>;

class import : public instruction {
public:
  import(std::string target, size_t line, size_t col)
      : instruction(line, col), target(target)
  {
  }
  import(size_t line, size_t col) : instruction(line, col) {}
  std::string target;
  virtual void visit(ins_receiver &v) override;
};
using import_ptr = std::unique_ptr<import>;

class function : public instruction {
public:
  function(size_t line, size_t col) : instruction(line, col) {}
  std::string name;
  std::string file_name;
  variable_ptr return_data;
  std::vector<variable_ptr> parameters;
  std::vector<instruction_ptr> instruction_list;
  virtual void visit(ins_receiver &v) override;
};
using function_ptr = std::unique_ptr<function>;

/*

   Receives instruction statements

 */
class ins_receiver {
public:
  virtual void receive(define_user_struct &ins) = 0;
  virtual void receive(assignment_instruction &ins) = 0;
  virtual void receive(expression_instruction &ins) = 0;
  virtual void receive(if_instruction &ins) = 0;
  virtual void receive(while_instruction &ins) = 0;
  virtual void receive(for_instruction &ins) = 0;
  virtual void receive(return_instruction &ins) = 0;
  virtual void receive(import &ins) = 0;
  virtual void receive(function &ins) = 0;
};

} // namespace instructions

} // namespace titan

#endif
