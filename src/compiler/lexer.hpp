#ifndef COMPILER_LEXER_HPP
#define COMPILER_LEXER_HPP

#include <set>
#include <string>
#include <vector>

#include "common.hpp"
#include "tokens.hpp"

namespace compiler {

class lexer {

public:
  enum class LexerResult { OKAY, WARNINGS, ERRORS };

  lexer();

  bool load_files(std::vector<std::string> &files);

  LexerResult lex(std::vector<TD_Pair> &tokens);

private:
  void clear();
  std::vector<SourceLine> _program_lines;
  std::vector<TD_Pair> *_tokens;
  SourceLine *_current_line;
  size_t _line_len;
  size_t _idx;
  std::set<char> _id_stop;

  LexerResult lex_line();

  void advance();
  char peek();
};
} // namespace compiler

#endif