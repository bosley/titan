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
        _tokens->emplace_back(TD_Pair{Token::FN, {}});
      }
      break;

    case '(':
      _tokens->emplace_back(TD_Pair{Token::L_PAREN, {}});
      break;

    case ')':
      _tokens->emplace_back(TD_Pair{Token::R_PAREN, {}});
      break;

    case '[':
      _tokens->emplace_back(TD_Pair{Token::L_BRACKET, {}});
      break;

    case ']':
      _tokens->emplace_back(TD_Pair{Token::R_BRACKET, {}});
      break;

    case '{':
      _tokens->emplace_back(TD_Pair{Token::L_BRACE, {}});
      break;

    case '}':
      _tokens->emplace_back(TD_Pair{Token::R_BRACE, {}});
      break;

    case ':':
      _tokens->emplace_back(TD_Pair{Token::COLON, {}});
      break;

    case ';':
      _tokens->emplace_back(TD_Pair{Token::SEMICOLON, {}});
      break;

    case ',':
      _tokens->emplace_back(TD_Pair{Token::COMMA, {}});
      break;

    case '>':
      _tokens->emplace_back(TD_Pair{Token::GT, {}});
      break;

    case '<':
      _tokens->emplace_back(TD_Pair{Token::LT, {}});
      break;

    case '@':
      _tokens->emplace_back(TD_Pair{Token::AT, {}});
      break;

    case '$':
      _tokens->emplace_back(TD_Pair{Token::DOLLAR, {}});
      break;

    case '"':
      _tokens->emplace_back(TD_Pair{Token::DOUBLE_QUOTE, {}});
      break;

    case '\'':
      _tokens->emplace_back(TD_Pair{Token::SINGLE_QUOTE, {}});
      break;

    case '?':
      _tokens->emplace_back(TD_Pair{Token::QUESTION_MARK, {}});
      break;

    case '.':
      _tokens->emplace_back(TD_Pair{Token::PERIOD, {}});
      break;

    case '#':
      _tokens->emplace_back(TD_Pair{Token::OCTOTHORPE, {}});
      break;

    case '!':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::EXCLAMATION_EQ, {}});
      } else {
        _tokens->emplace_back(TD_Pair{Token::EXCLAMATION, {}});
      }
      break;

    case '-':
      if (peek() == '>') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::ARROW, {}});
      } else if (peek() == '=') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::SUB_EQ, {}});
      } else {
        _tokens->emplace_back(TD_Pair{Token::SUB, {}});
      }
      break;

    case '+':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::ADD_EQ, {}});
      } else {
        _tokens->emplace_back(TD_Pair{Token::ADD, {}});
      }
      break;

    case '/':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::DIV_EQ, {}});
      } else {
        _tokens->emplace_back(TD_Pair{Token::DIV, {}});
      }
      break;

    case '*':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::MUL_EQ, {}});
      } else if (peek() == '*') {
        advance();
        if (peek() == '=') {
          advance();
          _tokens->emplace_back(TD_Pair{Token::POW_EQ, {}});
        } else {
          _tokens->emplace_back(TD_Pair{Token::POW, {}});
        }
      } else {
        _tokens->emplace_back(TD_Pair{Token::MUL, {}});
      }
      break;

    case '%':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::MOD_EQ, {}});
      } else {
        _tokens->emplace_back(TD_Pair{Token::MOD, {}});
      }
      break;

    case '&':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::AMPERSAND_EQ, {}});
      } else if (peek() == '&') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::AND, {}});
      } else {
        _tokens->emplace_back(TD_Pair{Token::AMPERSAND, {}});
      }
      break;

    case '|':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::PIPE_EQ, {}});
      } else if (peek() == '|') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::OR, {}});
      } else {
        _tokens->emplace_back(TD_Pair{Token::PIPE, {}});
      }
      break;

    case '~':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::TILDE_EQ, {}});
      } else {
        _tokens->emplace_back(TD_Pair{Token::TILDE, {}});
      }
      break;

    case '^':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::HAT_EQ, {}});
      } else {
        _tokens->emplace_back(TD_Pair{Token::HAT, {}});
      }
      break;

    case '=':
      if (peek() == '=') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::EQ_EQ, {}});
      } else {
        _tokens->emplace_back(TD_Pair{Token::EQ, {}});
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
          _tokens->emplace_back(TD_Pair{Token::LITERAL_FLOAT, item});
        } else {
          _tokens->emplace_back(TD_Pair{Token::LITERAL_NUMBER, item});
        }
        break;
      }

      // Eat identifier until next white space
      _tokens->emplace_back(TD_Pair{Token::IDENTIFIER, {}});
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