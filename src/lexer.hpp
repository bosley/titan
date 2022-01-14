#ifndef TITAN_LEXER_HPP
#define TITAN_LEXER_HPP

#include <string>
#include <vector>

#include "tokens.hpp"

namespace titan {

class lexer {

public:
  lexer();
  void clear();
  std::vector<TD_Pair> lex(size_t line_no, std::string line);

private:
  std::vector<TD_Pair> _tokens;
  struct SourceLine {
    std::string data;
    size_t line_no;
  };
  std::string _current_line;
  size_t _idx;

  void advance();
  char peek(size_t ahead = 1);
};
} // namespace titan

#endif
