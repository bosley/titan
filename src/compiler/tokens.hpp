#ifndef COMPILER_TOKENS_HPP
#define COMPILER_TOKENS_HPP

#include <string>

namespace compiler {

enum class Token {
  FN = 0,
  IDENTIFIER,
  L_PAREN,
  R_PAREN,
  L_BRACE,
  R_BRACE,
  L_BRACKET,
  R_BRACKET,
  COLON,
  SEMICOLON,
  COMMA,
  GT,
  LT,
  ARROW,
  SUB,
};

struct TD_Pair {
  Token token;
  std::string data;
};

static std::string token_to_str(const TD_Pair &td) {
  switch (td.token) {
  case Token::FN:
    return "FN";
  case Token::IDENTIFIER:
    return "IDENTIFIER(" + td.data + ")";
  case Token::L_BRACKET:
    return "L_BRACKET";
  case Token::R_BRACKET:
    return "R_BRACKET";
  case Token::L_PAREN:
    return "L_PAREN";
  case Token::R_PAREN:
    return "R_PAREN";
  case Token::L_BRACE:
    return "L_BRACE";
  case Token::R_BRACE:
    return "R_BRACE";
  case Token::COLON:
    return "COLON";
  case Token::SEMICOLON:
    return "SEMICOLON";
  case Token::COMMA:
    return "COMMA";
  case Token::GT:
    return "GT";
  case Token::LT:
    return "LT";
  case Token::SUB:
    return "SUB";
  case Token::ARROW:
    return "ARROW";
  }

  return "UNKNOWN";
}

} // namespace compiler

#endif