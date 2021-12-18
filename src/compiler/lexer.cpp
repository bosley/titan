#include "lexer.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace compiler {

lexer::lexer()
    : _tokens(nullptr), _current_line(nullptr), _line_len(0), _idx(0) {
  _id_stop.insert('\0');
  _id_stop.insert('[');
  _id_stop.insert('(');
  _id_stop.insert('{');
  _id_stop.insert(':');
  _id_stop.insert('=');
  _id_stop.insert(',');
  _id_stop.insert('-');
  _id_stop.insert('>');
}

bool lexer::load_files(std::vector<std::string> &files) {
  clear();

  constexpr auto trim_line = [](std::string &line) -> void {
    line.erase(std::find_if(line.rbegin(), line.rend(),
                            [](int ch) { return !std::isspace(ch); })
                   .base(),
               line.end());
    line.erase(line.begin(), find_if_not(line.begin(), line.end(),
                                         [](int c) { return isspace(c); }));
  };

  for (auto &file : files) {

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
  }

  return true;
}

lexer::LexerResult lexer::lex(std::vector<TD_Pair> &tokens) {

  tokens.clear();
  _tokens = &tokens;

  for (auto &current_line : _program_lines) {

    std::cout << current_line.origin_file << "(" << current_line.file_line_no
              << "): " << current_line.data << std::endl;

    _current_line = &current_line;

    auto result = lex_line();
    if (result != lexer::LexerResult::OKAY) {
      tokens.clear();
      clear();
      return result;
    }
  }

  return LexerResult::OKAY;
}

void lexer::clear() {
  _program_lines.clear();
  _current_line = nullptr;
  _tokens = nullptr;
  _line_len = 0;
  _idx = 0;
}

lexer::LexerResult lexer::lex_line() {

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

    case '-':
      if (peek() == '>') {
        advance();
        _tokens->emplace_back(TD_Pair{Token::ARROW, {}});
      } else {
        _tokens->emplace_back(TD_Pair{Token::SUB, {}});
      }
      break;

    default:
      if (isspace(_current_line->data[_idx])) {
        break;
      }

      // Eat identifier until next white space
      _tokens->emplace_back(TD_Pair{Token::IDENTIFIER, {}});
      for (; !isspace(peek()) && !_id_stop.contains(peek()); advance()) {
        _tokens->back().data += _current_line->data[_idx];
      }
      _tokens->back().data += _current_line->data[_idx];
      break;
    }
  }

  return LexerResult::OKAY;
}

void lexer::advance() { _idx++; }

char lexer::peek() {
  if (!_current_line) {
    return '\0';
  }
  if (_idx + 1 >= _line_len) {
    return '\0';
  }
  return _current_line->data[_idx + 1];
}

} // namespace compiler