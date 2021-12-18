#include "parser.hpp"
#include <iostream>

namespace compiler {

parser::parser() : _parser_okay(true), _idx(0), _tokens(nullptr) {}

std::vector<parse_tree::toplevel *>
parser::parse(std::function<std::vector<TD_Pair>(std::string)> import_file,
              std::vector<TD_Pair> &tokens) {
  _tokens = &tokens;
  std::vector<parse_tree::toplevel *> top_level_items;

  parse_tree::toplevel *new_top_level_item;

  while (_parser_okay && _idx < _tokens->size()) {

    /*
        Check fo an import statement
        If a statement is returned
    */
    new_top_level_item = import_stmt();
    if (new_top_level_item) {
      if (!_parser_okay) {
        continue;
      }

      parse_tree::import_stmt *import_statement =
          static_cast<parse_tree::import_stmt *>(new_top_level_item);

      // Ensure we haven't imported it yet
      if (_imported_objects.find(import_statement->target) !=
          _imported_objects.end()) {
        delete new_top_level_item;
        continue;
      }
      _imported_objects.insert(import_statement->target);

      // Lex and parse the file
      std::vector<TD_Pair> imported_tokens =
          import_file(import_statement->target);

      parser import_parser;
      std::vector<parse_tree::toplevel *> parsed_file =
          import_parser.parse(import_file, imported_tokens);
      if (!import_parser.is_okay()) {
        _parser_okay = true;
        continue;
      }

      // Add it to our top level objects
      top_level_items.insert(top_level_items.end(), parsed_file.begin(),
                             parsed_file.end());
    }
    if (!_parser_okay) {
      continue;
    }

    new_top_level_item = function();
    if (new_top_level_item) {
      top_level_items.push_back(new_top_level_item);
    }
    if (!_parser_okay) {
      continue;
    }

    // Add if statements here for other top level items

    // No top level items were found we must be done
    if (!new_top_level_item) {
      return top_level_items;
    }
  }

  if (!_parser_okay) {
    // If we got here there is an err so we clean up
    for (size_t i = 0; i < top_level_items.size(); i++) {
      delete top_level_items[i];
    }
    top_level_items.clear();
  }

  return top_level_items;
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

parse_tree::toplevel *parser::import_stmt() { return nullptr; }

parse_tree::toplevel *parser::function() {

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

  auto params = function_params();

  // Function validated, return it
  if (_parser_okay) {
    return new_func;
  }
  delete new_func;
  return nullptr;
}

std::vector<parse_tree::variable> parser::function_params() { return {}; }
parse_tree::element *parser::statement() { return nullptr; }
parse_tree::element *parser::assignment() { return nullptr; }
parse_tree::element *parser::if_statement() { return nullptr; }
parse_tree::element *parser::else_if_statement() { return nullptr; }
parse_tree::element *parser::else_statement() { return nullptr; }
parse_tree::element *parser::loop() { return nullptr; }
parse_tree::expr_node *parser::expression() { return nullptr; }
parse_tree::expr_node *parser::term() { return nullptr; }
parse_tree::expr_node *parser::factor() { return nullptr; }
parse_tree::expr_node *parser::primary() { return nullptr; }
parse_tree::expr_node *parser::function_call() { return nullptr; }
std::vector<parse_tree::expr_node *> parser::function_call_params() {
  return {};
}
std::string parser::identifier() { return {}; };

} // namespace compiler