#ifndef COMPILER_LEXER_HPP
#define COMPILER_LEXER_HPP

#include <string>
#include <vector>

#include "common.hpp"
#include "tokens.hpp"

namespace compiler {

class lexer {

public:
  lexer();

  bool load_file(std::string &file);

  bool lex(std::vector<TD_Pair> &tokens);

private:
  void clear();
  std::vector<SourceLine> _program_lines;
  std::vector<TD_Pair> *_tokens;
  SourceLine *_current_line;
  size_t _line_len;
  size_t _idx;

  bool lex_line();

  void advance();
  char peek(size_t ahead = 1);
};
} // namespace compiler

#endif