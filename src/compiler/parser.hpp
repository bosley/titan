#ifndef COMPILER_PARSER_HPP
#define COMPILER_PARSER_HPP

#include <set>
#include <string>
#include <vector>

#include "common.hpp"
#include "parsetree.hpp"
#include "tokens.hpp"

namespace compiler {

class parser {
public:
  parser();

  std::vector<parse_tree::toplevel *>
  parse(std::string filename, std::vector<std::string> &include_directories,
        std::function<std::vector<TD_Pair>(std::string)> import_function,
        std::vector<TD_Pair> &tokens);

  bool is_okay() const { return _parser_okay; }

private:
  bool _parser_okay;
  size_t _idx;
  std::vector<TD_Pair> *_tokens;
  std::set<std::string> _imported_objects;
  std::string _filename;
  void advance();
  void expect(Token token, std::string error, size_t ahead = 0);
  TD_Pair peek(size_t ahead = 1);
  parse_tree::toplevel *function();
  parse_tree::toplevel *import_stmt();
  std::vector<parse_tree::variable> function_params();
  std::vector<parse_tree::element *> statements();
  parse_tree::element *statement();
  parse_tree::element *assignment();
  parse_tree::element *if_statement();
  parse_tree::element *else_if_statement();
  parse_tree::element *else_statement();
  parse_tree::element *loop();
  parse_tree::element *expression_statement();
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