#include "lexer.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>

namespace compiler {

lexer::lexer()
    : _tokens(nullptr), _current_line(nullptr), _line_len(0), _idx(0) {}

bool lexer::load_file(std::string &file) {
  clear();

  constexpr auto trim_line = [](std::string &line) -> void {
    line.erase(std::find_if(line.rbegin(), line.rend(),
                            [](int ch) { return !std::isspace(ch); })
                   .base(),
               line.end());
    line.erase(line.begin(), find_if_not(line.begin(), line.end(),
                                         [](int c) { return isspace(c); }));
  };

  if (!std::filesystem::is_regular_file(file)) {
    std::cout << "Given item is not a file : " << file << std::endl;
    return false;
  }

  std::ifstream ifs;
  ifs.open(file);

  if (!ifs.is_open()) {
    std::cout << "Unable to open file : " << file << std::endl;
    return false;
  }

  size_t line_no = 0;
  std::string line;
  while (std::getline(ifs, line)) {
    line_no++;

    trim_line(line);

    if (line.empty()) {
      continue;
    }

    // Comments
    if (line.size() >= 2 && line[0] == '/' && line[1] == '/') {
      continue;
    }

    _program_lines.emplace_back(SourceLine{line, line_no, file});
  }
  ifs.close();
  return true;
}

bool lexer::lex(std::vector<TD_Pair> &tokens) {
  tokens.clear();
  _tokens = &tokens;
  for (auto &current_line : _program_lines) {
    _current_line = &current_line;
    if (!lex_line()) {
      tokens.clear();
      clear();
      return false;
    }
  }
  return true;
}

void lexer::clear() {
  _program_lines.clear();
  _current_line = nullptr;
  _tokens = nullptr;
  _line_len = 0;
  _idx = 0;
}

bool lexer::lex_line() {

  _line_len = _current_line->data.size();
  for (_idx = 0; _idx < _line_len; advance()) {
    switch (_current_line->data[_idx]) {
    case 'f':
      if (peek() == 'n') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::FN, {}, &_current_line->file_line_no});
      }
      break;

    case '(':
      _tokens->emplace_back(
          TD_Pair{Token::L_PAREN, {}, &_current_line->file_line_no});
      break;

    case ')':
      _tokens->emplace_back(
          TD_Pair{Token::R_PAREN, {}, &_current_line->file_line_no});
      break;

    case '[':
      _tokens->emplace_back(
          TD_Pair{Token::L_BRACKET, {}, &_current_line->file_line_no});
      break;

    case ']':
      _tokens->emplace_back(
          TD_Pair{Token::R_BRACKET, {}, &_current_line->file_line_no});
      break;

    case '{':
      _tokens->emplace_back(
          TD_Pair{Token::L_BRACE, {}, &_current_line->file_line_no});
      break;

    case '}':
      _tokens->emplace_back(
          TD_Pair{Token::R_BRACE, {}, &_current_line->file_line_no});
      break;

    case ':':
      _tokens->emplace_back(
          TD_Pair{Token::COLON, {}, &_current_line->file_line_no});
      break;

    case ';':
      _tokens->emplace_back(
          TD_Pair{Token::SEMICOLON, {}, &_current_line->file_line_no});
      break;

    case ',':
      _tokens->emplace_back(
          TD_Pair{Token::COMMA, {}, &_current_line->file_line_no});
      break;

    case '>':
      _tokens->emplace_back(
          TD_Pair{Token::GT, {}, &_current_line->file_line_no});
      break;

    case '<':
      _tokens->emplace_back(
          TD_Pair{Token::LT, {}, &_current_line->file_line_no});
      break;

    case '@':
      _tokens->emplace_back(
          TD_Pair{Token::AT, {}, &_current_line->file_line_no});
      break;

    case '$':
      _tokens->emplace_back(
          TD_Pair{Token::DOLLAR, {}, &_current_line->file_line_no});
      break;

    case '"':
      _tokens->emplace_back(
          TD_Pair{Token::DOUBLE_QUOTE, {}, &_current_line->file_line_no});
      break;

    case '\'':
      _tokens->emplace_back(
          TD_Pair{Token::SINGLE_QUOTE, {}, &_current_line->file_line_no});
      break;

    case '?':
      _tokens->emplace_back(
          TD_Pair{Token::QUESTION_MARK, {}, &_current_line->file_line_no});
      break;

    case '.':
      _tokens->emplace_back(
          TD_Pair{Token::PERIOD, {}, &_current_line->file_line_no});
      break;

    case '#':
      _tokens->emplace_back(
          TD_Pair{Token::OCTOTHORPE, {}, &_current_line->file_line_no});
      break;

    case '!':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::EXCLAMATION_EQ, {}, &_current_line->file_line_no});
      } else {
        _tokens->emplace_back(
            TD_Pair{Token::EXCLAMATION, {}, &_current_line->file_line_no});
      }
      break;

    case '-':
      if (peek() == '>') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::ARROW, {}, &_current_line->file_line_no});
      } else if (peek() == '=') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::SUB_EQ, {}, &_current_line->file_line_no});
      } else {
        _tokens->emplace_back(
            TD_Pair{Token::SUB, {}, &_current_line->file_line_no});
      }
      break;

    case '+':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::ADD_EQ, {}, &_current_line->file_line_no});
      } else {
        _tokens->emplace_back(
            TD_Pair{Token::ADD, {}, &_current_line->file_line_no});
      }
      break;

    case '/':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::DIV_EQ, {}, &_current_line->file_line_no});
      } else {
        _tokens->emplace_back(
            TD_Pair{Token::DIV, {}, &_current_line->file_line_no});
      }
      break;

    case '*':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::MUL_EQ, {}, &_current_line->file_line_no});
      } else if (peek() == '*') {
        advance();
        if (peek() == '=') {
          advance();
          _tokens->emplace_back(
              TD_Pair{Token::POW_EQ, {}, &_current_line->file_line_no});
        } else {
          _tokens->emplace_back(
              TD_Pair{Token::POW, {}, &_current_line->file_line_no});
        }
      } else {
        _tokens->emplace_back(
            TD_Pair{Token::MUL, {}, &_current_line->file_line_no});
      }
      break;

    case '%':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::MOD_EQ, {}, &_current_line->file_line_no});
      } else {
        _tokens->emplace_back(
            TD_Pair{Token::MOD, {}, &_current_line->file_line_no});
      }
      break;

    case '&':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::AMPERSAND_EQ, {}, &_current_line->file_line_no});
      } else if (peek() == '&') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::AND, {}, &_current_line->file_line_no});
      } else {
        _tokens->emplace_back(
            TD_Pair{Token::AMPERSAND, {}, &_current_line->file_line_no});
      }
      break;

    case '|':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::PIPE_EQ, {}, &_current_line->file_line_no});
      } else if (peek() == '|') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::OR, {}, &_current_line->file_line_no});
      } else {
        _tokens->emplace_back(
            TD_Pair{Token::PIPE, {}, &_current_line->file_line_no});
      }
      break;

    case '~':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::TILDE_EQ, {}, &_current_line->file_line_no});
      } else {
        _tokens->emplace_back(
            TD_Pair{Token::TILDE, {}, &_current_line->file_line_no});
      }
      break;

    case '^':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::HAT_EQ, {}, &_current_line->file_line_no});
      } else {
        _tokens->emplace_back(
            TD_Pair{Token::HAT, {}, &_current_line->file_line_no});
      }
      break;

    case '=':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(
            TD_Pair{Token::EQ_EQ, {}, &_current_line->file_line_no});
      } else {
        _tokens->emplace_back(
            TD_Pair{Token::EQ, {}, &_current_line->file_line_no});
      }
      break;

    default:
      if (isspace(_current_line->data[_idx])) {
        break;
      }

      if (std::isdigit(_current_line->data[_idx])) {
        bool is_float = false;
        std::string item;
        while (std::isdigit(peek()) || (!is_float && peek() == '.')) {
          item += _current_line->data[_idx];
          if (_current_line->data[_idx] == '.') {
            is_float = true;
          }
          advance();
        }

        if (is_float) {
          _tokens->emplace_back(TD_Pair{Token::LITERAL_FLOAT, item,
                                        &_current_line->file_line_no});
        } else {
          _tokens->emplace_back(TD_Pair{Token::LITERAL_NUMBER, item,
                                        &_current_line->file_line_no});
        }
        break;
      }

      // Eat identifier until next white space
      _tokens->emplace_back(
          TD_Pair{Token::IDENTIFIER, {}, &_current_line->file_line_no});
      while (std::isalnum(peek()) || std::isdigit(peek()) || peek() == '_') {
        _tokens->back().data += _current_line->data[_idx];
        advance();
      }
      _tokens->back().data += _current_line->data[_idx];
      break;
    }
  }

  return true;
}

void lexer::advance() { _idx++; }

char lexer::peek(size_t ahead) {
  if (!_current_line) {
    return '\0';
  }
  if (_idx + ahead >= _line_len) {
    return '\0';
  }
  return _current_line->data[_idx + ahead];
}

} // namespace compiler