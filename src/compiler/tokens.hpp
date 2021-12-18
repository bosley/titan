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
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,
  POW,
  ADD_EQ,
  SUB_EQ,
  MUL_EQ,
  DIV_EQ,
  MOD_EQ,
  POW_EQ,
  AMPERSAND,
  PIPE,
  TILDE,
  HAT,
  AMPERSAND_EQ,
  PIPE_EQ,
  TILDE_EQ,
  HAT_EQ,
  EQ,
  EQ_EQ,
  AT,
  DOLLAR,
  DOUBLE_QUOTE,
  SINGLE_QUOTE,
  QUESTION_MARK,
  PERIOD,
  OCTOTHORPE,
  LITERAL_FLOAT,
  LITERAL_NUMBER,
  OR,
  AND,
  EXCLAMATION,
  EXCLAMATION_EQ,
  EOS // End of stream
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
  case Token::ADD:
    return "ADD";
  case Token::MUL:
    return "MUL";
  case Token::DIV:
    return "DIV";
  case Token::MOD:
    return "MOD";
  case Token::POW:
    return "POW";
  case Token::ADD_EQ:
    return "ADD_EQ";
  case Token::SUB_EQ:
    return "SUB_EQ";
  case Token::MUL_EQ:
    return "MUL_EQ";
  case Token::DIV_EQ:
    return "DIV_EQ";
  case Token::MOD_EQ:
    return "MOD_EQ";
  case Token::POW_EQ:
    return "POW_EQ";
  case Token::AMPERSAND:
    return "AMPERSAND";
  case Token::PIPE:
    return "PIPE";
  case Token::TILDE:
    return "TILDE";
  case Token::HAT:
    return "HAT";
  case Token::AMPERSAND_EQ:
    return "AMPERSAND_EQ";
  case Token::PIPE_EQ:
    return "PIPE_EQ";
  case Token::TILDE_EQ:
    return "TILDE_EQ";
  case Token::HAT_EQ:
    return "HAT_EQ";
  case Token::EQ:
    return "EQ";
  case Token::EQ_EQ:
    return "EQ_EQ";
  case Token::AT:
    return "AT";
  case Token::DOLLAR:
    return "DOLLAR";
  case Token::DOUBLE_QUOTE:
    return "DOUBLE_QUOTE";
  case Token::SINGLE_QUOTE:
    return "SINGLE_QUOTE";
  case Token::QUESTION_MARK:
    return "QUESTION_MARK";
  case Token::PERIOD:
    return "PERIOD";
  case Token::OCTOTHORPE:
    return "OCTOTHORPE";
  case Token::LITERAL_FLOAT:
    return "FLOAT(" + td.data + ")";
  case Token::LITERAL_NUMBER:
    return "NUMBER(" + td.data + ")";
  case Token::OR:
    return "OR";
  case Token::AND:
    return "AND";
  case Token::EXCLAMATION:
    return "EXCLAMATION";
  case Token::EXCLAMATION_EQ:
    return "EXCLAMATION_EQ";
  case Token::EOS:
    return "EOS";
  }

  return "UNKNOWN";
}

} // namespace compiler

#endif