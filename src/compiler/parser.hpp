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
  parse_tree::toplevel *parse_function();
  std::vector<parse_tree::variable> parse_function_parameters();
};
} // namespace compiler

#endif