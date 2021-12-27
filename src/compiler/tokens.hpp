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
  LTE,
  GTE,
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
  STRING,
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
  WHILE,
  FOR,
  IF,
  ELSE,
  RETURN,
  BREAK,
  LET,
  IMPORT,
  LSH,
  RSH,
  ERT, // Error token
  EOS  // End of stream
};

struct TD_Pair {
  Token token;
  std::string data;
  size_t *line;
};

static std::string token_to_str(const TD_Pair &td)
{

  size_t line_no = (td.line) ? *td.line : 0;
  switch (td.token) {
  case Token::FN:
    return "FN[" + std::to_string(line_no) + "]";
  case Token::IDENTIFIER:
    return "IDENTIFIER(" + td.data + ")[" + std::to_string(line_no) + "]";
  case Token::L_BRACKET:
    return "L_BRACKET[" + std::to_string(line_no) + "]";
  case Token::R_BRACKET:
    return "R_BRACKET[" + std::to_string(line_no) + "]";
  case Token::L_PAREN:
    return "L_PAREN[" + std::to_string(line_no) + "]";
  case Token::R_PAREN:
    return "R_PAREN[" + std::to_string(line_no) + "]";
  case Token::L_BRACE:
    return "L_BRACE[" + std::to_string(line_no) + "]";
  case Token::R_BRACE:
    return "R_BRACE[" + std::to_string(line_no) + "]";
  case Token::COLON:
    return "COLON[" + std::to_string(line_no) + "]";
  case Token::SEMICOLON:
    return "SEMICOLON[" + std::to_string(line_no) + "]";
  case Token::COMMA:
    return "COMMA[" + std::to_string(line_no) + "]";
  case Token::GT:
    return "GT[" + std::to_string(line_no) + "]";
  case Token::LT:
    return "LT[" + std::to_string(line_no) + "]";
  case Token::GTE:
    return "GTE[" + std::to_string(line_no) + "]";
  case Token::LTE:
    return "LTE[" + std::to_string(line_no) + "]";
  case Token::SUB:
    return "SUB[" + std::to_string(line_no) + "]";
  case Token::ARROW:
    return "ARROW[" + std::to_string(line_no) + "]";
  case Token::ADD:
    return "ADD[" + std::to_string(line_no) + "]";
  case Token::MUL:
    return "MUL[" + std::to_string(line_no) + "]";
  case Token::DIV:
    return "DIV[" + std::to_string(line_no) + "]";
  case Token::MOD:
    return "MOD[" + std::to_string(line_no) + "]";
  case Token::POW:
    return "POW[" + std::to_string(line_no) + "]";
  case Token::ADD_EQ:
    return "ADD_EQ[" + std::to_string(line_no) + "]";
  case Token::SUB_EQ:
    return "SUB_EQ[" + std::to_string(line_no) + "]";
  case Token::MUL_EQ:
    return "MUL_EQ[" + std::to_string(line_no) + "]";
  case Token::DIV_EQ:
    return "DIV_EQ[" + std::to_string(line_no) + "]";
  case Token::MOD_EQ:
    return "MOD_EQ[" + std::to_string(line_no) + "]";
  case Token::POW_EQ:
    return "POW_EQ[" + std::to_string(line_no) + "]";
  case Token::AMPERSAND:
    return "AMPERSAND[" + std::to_string(line_no) + "]";
  case Token::PIPE:
    return "PIPE[" + std::to_string(line_no) + "]";
  case Token::TILDE:
    return "TILDE[" + std::to_string(line_no) + "]";
  case Token::HAT:
    return "HAT[" + std::to_string(line_no) + "]";
  case Token::AMPERSAND_EQ:
    return "AMPERSAND_EQ[" + std::to_string(line_no) + "]";
  case Token::PIPE_EQ:
    return "PIPE_EQ[" + std::to_string(line_no) + "]";
  case Token::TILDE_EQ:
    return "TILDE_EQ[" + std::to_string(line_no) + "]";
  case Token::HAT_EQ:
    return "HAT_EQ[" + std::to_string(line_no) + "]";
  case Token::EQ:
    return "EQ[" + std::to_string(line_no) + "]";
  case Token::EQ_EQ:
    return "EQ_EQ[" + std::to_string(line_no) + "]";
  case Token::AT:
    return "AT[" + std::to_string(line_no) + "]";
  case Token::DOLLAR:
    return "DOLLAR[" + std::to_string(line_no) + "]";
  case Token::STRING:
    return "STRING(" + td.data + ")[" + std::to_string(line_no) + "]";
  case Token::SINGLE_QUOTE:
    return "SINGLE_QUOTE[" + std::to_string(line_no) + "]";
  case Token::QUESTION_MARK:
    return "QUESTION_MARK[" + std::to_string(line_no) + "]";
  case Token::PERIOD:
    return "PERIOD[" + std::to_string(line_no) + "]";
  case Token::OCTOTHORPE:
    return "OCTOTHORPE[" + std::to_string(line_no) + "]";
  case Token::LITERAL_FLOAT:
    return "FLOAT(" + td.data + ")[" + std::to_string(line_no) + "]";
  case Token::LITERAL_NUMBER:
    return "NUMBER(" + td.data + ")[" + std::to_string(line_no) + "]";
  case Token::OR:
    return "OR[" + std::to_string(line_no) + "]";
  case Token::AND:
    return "AND[" + std::to_string(line_no) + "]";
  case Token::EXCLAMATION:
    return "EXCLAMATION[" + std::to_string(line_no) + "]";
  case Token::EXCLAMATION_EQ:
    return "EXCLAMATION_EQ[" + std::to_string(line_no) + "]";
  case Token::WHILE:
    return "WHILE[" + std::to_string(line_no) + "]";
  case Token::FOR:
    return "FOR[" + std::to_string(line_no) + "]";
  case Token::IF:
    return "IF[" + std::to_string(line_no) + "]";
  case Token::ELSE:
    return "ELSE[" + std::to_string(line_no) + "]";
  case Token::RETURN:
    return "RETURN[" + std::to_string(line_no) + "]";
  case Token::BREAK:
    return "BREAK[" + std::to_string(line_no) + "]";
  case Token::LET:
    return "LET[" + std::to_string(line_no) + "]";
  case Token::IMPORT:
    return "IMPORT[" + std::to_string(line_no) + "]";
  case Token::LSH:
    return "LSH[" + std::to_string(line_no) + "]";
  case Token::RSH:
    return "RSH[" + std::to_string(line_no) + "]";
  case Token::EOS:
    return "EOS[" + std::to_string(line_no) + "]";
  case Token::ERT:
    return "ERT";
  }

  return "UNKNOWN[" + std::to_string(line_no) + "]";
}

} // namespace compiler

#endif
