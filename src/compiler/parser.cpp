#include "parser.hpp"
#include <iostream>

namespace compiler {

parser::parser() : _parser_okay(true), _idx(0), _tokens(nullptr) {}

std::vector<parse_tree::toplevel *>
parser::parse(std::vector<TD_Pair> &tokens) {
  _tokens = &tokens;
  std::vector<parse_tree::toplevel *> top_level_items;

  parse_tree::toplevel *new_top_level_item;

  while (_parser_okay && _idx < _tokens->size()) {

    new_top_level_item = parse_function();
    if (new_top_level_item) {
      top_level_items.push_back(new_top_level_item);
    }
    if (!_parser_okay) { // Skip ahead to exit for failure
      continue;
    }

    // Add if statements here for other top level items

    // No top level items were found we must be done
    if (!new_top_level_item) {
      return top_level_items;
    }
  }

  // If we got here there is an err so we clean up
  for (size_t i = 0; i < top_level_items.size(); i++) {
    delete top_level_items[i];
  }

  return {};
}

void parser::advance() { _idx++; }

TD_Pair parser::peek(size_t ahead) {
  if (!_tokens) {
    return TD_Pair{Token::EOS, {}};
  }
  if (_idx + ahead >= _tokens->size()) {
    return TD_Pair{Token::EOS, {}};
  }
  return _tokens->at(_idx + ahead);
}

parse_tree::toplevel *parser::parse_function() {

  // fn some_function
  if (_tokens->at(_idx).token != Token::FN) {
    return nullptr;
  }

  parse_tree::function *new_func = new parse_tree::function();

  /*

    TODO: Start here (cli currently hangs until we eat the function or : set
    _parser_okay = false;

  */

  // Get function name

  auto params = parse_function_parameters();

  // Function validated, return it
  if (_parser_okay) {
    return new_func;
  }
  delete new_func;
  return nullptr;
}

std::vector<parse_tree::variable> parser::parse_function_parameters() {
  return {};
}

} // namespace compiler