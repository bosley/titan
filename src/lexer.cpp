#include "lexer.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>

namespace titan {

lexer::lexer(): _idx(0)
{
}

void lexer::clear()
{
  _tokens.clear();
  _current_line.clear();
  _idx = 0;
}

std::vector<TD_Pair> lexer::lex(size_t line_no, std::string line) 
{
  _current_line = line;

  for (_idx = 0; _idx < _current_line.size(); advance()) {
    switch (_current_line[_idx]) {
    case '(':
      _tokens.emplace_back(
          TD_Pair{Token::L_PAREN, "(", line_no, _idx});
      break;

    case ')':
      _tokens.emplace_back(
          TD_Pair{Token::R_PAREN, ")", line_no, _idx});
      break;

    case '[':
      _tokens.emplace_back(
          TD_Pair{Token::L_BRACKET, "[", line_no, _idx});
      break;

    case ']':
      _tokens.emplace_back(
          TD_Pair{Token::R_BRACKET, "]", line_no, _idx});
      break;

    case '{':
      _tokens.emplace_back(
          TD_Pair{Token::L_BRACE, "{", line_no, _idx});
      break;

    case '}':
      _tokens.emplace_back(
          TD_Pair{Token::R_BRACE, "}", line_no, _idx});
      break;

    case ':':
      _tokens.emplace_back(
          TD_Pair{Token::COLON, ":", line_no, _idx});
      break;

    case ';':
      _tokens.emplace_back(
          TD_Pair{Token::SEMICOLON, ";", line_no, _idx});
      break;

    case ',':
      _tokens.emplace_back(
          TD_Pair{Token::COMMA, ",", line_no, _idx});
      break;

    case '>':
      if (peek() == '>') {
        advance();
        if(peek() == '=') {
          advance();
          _tokens.emplace_back(
              TD_Pair{Token::RSH_EQ, ">>=", line_no, _idx});
        } 
        else {
          _tokens.emplace_back(
              TD_Pair{Token::RSH, ">>", line_no, _idx});
        }
      }
      else if (peek() == '=') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::GTE, ">=", line_no, _idx});
      }
      else {
        _tokens.emplace_back(
            TD_Pair{Token::GT, ">", line_no, _idx});
      }
      break;

    case '<':
      if (peek() == '<') {
        advance();
        if(peek() == '=') {
          advance();
          _tokens.emplace_back(
              TD_Pair{Token::LSH_EQ, "<<=", line_no, _idx});
        } else {
          _tokens.emplace_back(
              TD_Pair{Token::LSH, "<<", line_no, _idx});
        }
      }
      else if (peek() == '=') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::LTE, "<=", line_no, _idx});
      }
      else {
        _tokens.emplace_back(
            TD_Pair{Token::LT, "<", line_no, _idx});
      }
      break;

    case '@':
      _tokens.emplace_back(
          TD_Pair{Token::AT, "@", line_no, _idx});
      break;

    case '$':
      _tokens.emplace_back(
          TD_Pair{Token::DOLLAR, "$", line_no, _idx});
      break;

    case '\'':
      _tokens.emplace_back(
          TD_Pair{Token::SINGLE_QUOTE, "'", line_no, _idx});
      break;

    case '?':
      _tokens.emplace_back(TD_Pair{Token::QUESTION_MARK, "?",
                                    line_no, _idx});
      break;

    case '.':
      _tokens.emplace_back(
          TD_Pair{Token::PERIOD, ".", line_no, _idx});
      break;

    case '#':
      _tokens.emplace_back(
          TD_Pair{Token::OCTOTHORPE, "#", line_no, _idx});
      break;

    case '!':
      if (peek() == '=') {
        advance();
        _tokens.emplace_back(TD_Pair{Token::EXCLAMATION_EQ,
                                      "!=", line_no, _idx});
      }
      else {
        _tokens.emplace_back(TD_Pair{Token::EXCLAMATION, "!",
                                      line_no, _idx});
      }
      break;

    case '-':
      if (peek() == '>') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::ARROW, "->", line_no, _idx});
      }
      else if (peek() == '=') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::SUB_EQ, "-=", line_no, _idx});
      }
      else {
        _tokens.emplace_back(
            TD_Pair{Token::SUB, "-", line_no, _idx});
      }
      break;

    case '+':
      if (peek() == '=') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::ADD_EQ, "+=", line_no, _idx});
      }
      else {
        _tokens.emplace_back(
            TD_Pair{Token::ADD, "+", line_no, _idx});
      }
      break;

    case '/':
      if (peek() == '=') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::DIV_EQ, "/=", line_no, _idx});
      }
      else {
        _tokens.emplace_back(
            TD_Pair{Token::DIV, "/", line_no, _idx});
      }
      break;

    case '*':
      if (peek() == '=') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::MUL_EQ, "*=", line_no, _idx});
      }
      else if (peek() == '*') {
        advance();
        if (peek() == '=') {
          advance();
          _tokens.emplace_back(
              TD_Pair{Token::POW_EQ, "**=", line_no, _idx});
        }
        else {
          _tokens.emplace_back(
              TD_Pair{Token::POW, "**", line_no, _idx});
        }
      }
      else {
        _tokens.emplace_back(
            TD_Pair{Token::MUL, "*", line_no, _idx});
      }
      break;

    case '%':
      if (peek() == '=') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::MOD_EQ, "%=", line_no, _idx});
      }
      else {
        _tokens.emplace_back(
            TD_Pair{Token::MOD, "%", line_no, _idx});
      }
      break;

    case '&':
      if (peek() == '=') {
        advance();
        _tokens.emplace_back(TD_Pair{Token::AMPERSAND_EQ,
                                      "&=", line_no, _idx});
      }
      else if (peek() == '&') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::AND, "&&", line_no, _idx});
      }
      else {
        _tokens.emplace_back(
            TD_Pair{Token::AMPERSAND, "&", line_no, _idx});
      }
      break;

    case '|':
      if (peek() == '=') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::PIPE_EQ, "|=", line_no, _idx});
      }
      else if (peek() == '|') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::OR, "||", line_no, _idx});
      }
      else {
        _tokens.emplace_back(
            TD_Pair{Token::PIPE, "|", line_no, _idx});
      }
      break;

    case '~':
      if (peek() == '=') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::TILDE_EQ, "~=", line_no, _idx});
      }
      else {
        _tokens.emplace_back(
            TD_Pair{Token::TILDE, "~", line_no, _idx});
      }
      break;

    case '^':
      if (peek() == '=') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::HAT_EQ, "^=", line_no, _idx});
      }
      else {
        _tokens.emplace_back(
            TD_Pair{Token::HAT, "^", line_no, _idx});
      }
      break;

    case '=':
      if (peek() == '=') {
        advance();
        _tokens.emplace_back(
            TD_Pair{Token::EQ_EQ, "==", line_no, _idx});
      }
      else {
        _tokens.emplace_back(
            TD_Pair{Token::EQ, "=", line_no, _idx});
      }
      break;

    case '"': {
      std::string value;
      bool consume_string = true;
      while (consume_string && peek() != '\0') {
        if ('\\' == _current_line[_idx]) {
          value += _current_line[_idx];
          advance();
          if ('\"' == _current_line[_idx]) {
            value += _current_line[_idx];
            advance();
            continue;
          }
        }
        if ('"' == peek()) {
          consume_string = false;
          value += _current_line[_idx];
          advance();
          continue;
        }
        if ('"' != _current_line[_idx]) {
          value += _current_line[_idx];
        }
        advance();
      }

      _tokens.emplace_back(
          TD_Pair{Token::STRING, value, line_no, _idx});
      break;
    }
    default:
      if (isspace(_current_line[_idx])) {
        break;
      }

      if (std::isdigit(_current_line[_idx])) {
        bool is_float = false;
        std::string item;
        while (std::isdigit(peek()) || (!is_float && peek() == '.')) {
          item += _current_line[_idx];
          if (_current_line[_idx] == '.') {
            is_float = true;
          }
          advance();
        }
        item += _current_line[_idx];

        if (is_float) {
          _tokens.emplace_back(TD_Pair{Token::LITERAL_FLOAT, item,
                                        line_no, _idx});
        }
        else {
          _tokens.emplace_back(TD_Pair{Token::LITERAL_NUMBER, item,
                                        line_no, _idx});
        }
        break;
      }

      // Eat some word, could be a reserved word or an identifier
      std::string word;
      while (std::isalnum(peek()) || std::isdigit(peek()) || peek() == '_') {
        word += _current_line[_idx];
        advance();
      }
      word += _current_line[_idx];

      // Check against reserved words, default to assuming its an identifier
      if (word == "fn") {
        _tokens.emplace_back(
            TD_Pair{Token::FN, {}, line_no, _idx});
      }
      else if (word == "while") {
        _tokens.emplace_back(
            TD_Pair{Token::WHILE, {}, line_no, _idx});
      }
      else if (word == "for") {
        _tokens.emplace_back(
            TD_Pair{Token::FOR, {}, line_no, _idx});
      }
      else if (word == "if") {
        _tokens.emplace_back(
            TD_Pair{Token::IF, {}, line_no, _idx});
      }
      else if (word == "else") {
        _tokens.emplace_back(
            TD_Pair{Token::ELSE, {}, line_no, _idx});
      }
      else if (word == "return") {
        _tokens.emplace_back(
            TD_Pair{Token::RETURN, {}, line_no, _idx});
      }
      else if (word == "break") {
        _tokens.emplace_back(
            TD_Pair{Token::BREAK, {}, line_no, _idx});
      }
      else if (word == "let") {
        _tokens.emplace_back(
            TD_Pair{Token::LET, {}, line_no, _idx});
      }
      else if (word == "import") {
        _tokens.emplace_back(
            TD_Pair{Token::IMPORT, {}, line_no, _idx});
      }
      else {
        _tokens.emplace_back(TD_Pair{Token::IDENTIFIER, word,
                                      line_no, _idx});
      }
      break;
    }
  }

  return _tokens;
}

void lexer::advance() { _idx++; }

char lexer::peek(size_t ahead)
{
  if (_current_line.empty()) {
    return '\0';
  }
  if (_idx + ahead >= _current_line.size()) {
    return '\0';
  }
  return _current_line[_idx + ahead];
}

} // namespace compiler
