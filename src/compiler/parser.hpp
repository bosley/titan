#ifndef COMPILER_PARSER_HPP
#define COMPILER_PARSER_HPP

#include <string>
#include <vector>

#include "common.hpp"
#include "parsetree.hpp"
#include "tokens.hpp"

namespace compiler {

class parser {
public:
  parser();

  std::vector<parse_tree::toplevel *> parse(std::vector<TD_Pair> &tokens);

private:
  bool _parser_okay;
  size_t _idx;
  std::vector<TD_Pair> *_tokens;
  void advance();
  TD_Pair peek(size_t ahead = 1);
  parse_tree::toplevel *function();
  std::vector<parse_tree::variable> function_params();
  std::vector<parse_tree::element *> statements();
  parse_tree::element *statement();
  parse_tree::element *assignment();
  parse_tree::element *if_statement();
  parse_tree::element *else_if_statement();
  parse_tree::element *else_statement();
  parse_tree::element *loop();
  parse_tree::expr_node *expression();
  parse_tree::expr_node *term();
  parse_tree::expr_node *factor();
  parse_tree::expr_node *primary();
  parse_tree::expr_node *function_call();
  std::vector<parse_tree::expr_node *> function_call_params();
  std::string identifier();
};
} // namespace compiler

#endif