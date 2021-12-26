#include "parser.hpp"

#include <filesystem>
#include <iostream>
#include <tuple>
#include <unordered_map>

namespace compiler {

namespace {

std::unordered_map<Token, parser::precedence> precedences = {
    {Token::EQ_EQ, parser::precedence::EQUALS},
    {Token::EXCLAMATION_EQ, parser::precedence::EQUALS},
    {Token::LT, parser::precedence::LESS_GREATER},
    {Token::GT, parser::precedence::LESS_GREATER},
    {Token::LTE, parser::precedence::LESS_GREATER},
    {Token::GTE, parser::precedence::LESS_GREATER},
    {Token::ADD, parser::precedence::SUM},
    {Token::SUB, parser::precedence::SUM},
    {Token::DIV, parser::precedence::PROD},
    {Token::MUL, parser::precedence::PROD},
    {Token::MOD, parser::precedence::PROD},
    {Token::L_PAREN, parser::precedence::CALL},
    {Token::L_BRACKET, parser::precedence::INDEX},
};

/* Stores files found [import target] => [location found from include dir] */
static std::unordered_map<std::string, std::string> located_items;

/* Finds an import */
static std::tuple<bool, std::string>
locate_import(std::vector<std::string> &paths, std::string &target) {

  // Check the local directory first
  std::filesystem::path item_as_local = std::filesystem::current_path();
  item_as_local /= target;
  if (std::filesystem::is_regular_file(item_as_local)) {
    return {true, item_as_local};
  }

  // If its not in the same directory as the file importing it, check the
  // already located items
  if (located_items.end() != located_items.find(target)) {
    return {true, located_items[target]};
  }

  // If it still isn't found, we need to iterate all include dirs and search for
  // it
  for (auto &dir : paths) {
    std::filesystem::path item_path = dir;
    item_path /= target;

    // If we find the item store it for later before handing off
    if (std::filesystem::is_regular_file(item_path)) {
      located_items[target] = item_path;
      return {true, item_path};
    }
  }

  return {false, {}};
}

static void report_error(const std::string &filename, size_t *line,
                         const std::string error) {
  std::cout << "Parse error [" << filename << "](" << *line << ") : " << error
            << std::endl;
}
} // namespace

parser::parser() : _parser_okay(true), _idx(0), _tokens(nullptr) {}

std::vector<parse_tree::toplevel *>
parser::parse(std::string filename,
              std::vector<std::string> &include_directories,
              std::function<std::vector<TD_Pair>(std::string)> import_file,
              std::vector<TD_Pair> &tokens) {
  _tokens = &tokens;
  _filename = filename;

  _prefix_fns[Token::IDENTIFIER] = &parser::identifier;
  _prefix_fns[Token::LITERAL_NUMBER] = &parser::number;
  _prefix_fns[Token::LITERAL_FLOAT] = &parser::number;
  _prefix_fns[Token::STRING] = &parser::str;
  _prefix_fns[Token::EXCLAMATION] = &parser::prefix_expr;
  _prefix_fns[Token::SUB] = &parser::prefix_expr;
  _prefix_fns[Token::L_PAREN] = &parser::grouped_expr;
  _prefix_fns[Token::L_BRACKET] = &parser::array;

  _infix_fns[Token::ADD] = &parser::infix_expr;
  _infix_fns[Token::SUB] = &parser::infix_expr;
  _infix_fns[Token::DIV] = &parser::infix_expr;
  _infix_fns[Token::MUL] = &parser::infix_expr;
  _infix_fns[Token::MOD] = &parser::infix_expr;
  _infix_fns[Token::EQ_EQ] = &parser::infix_expr;
  _infix_fns[Token::EXCLAMATION_EQ] = &parser::infix_expr;
  _infix_fns[Token::LT] = &parser::infix_expr;
  _infix_fns[Token::LTE] = &parser::infix_expr;
  _infix_fns[Token::GT] = &parser::infix_expr;
  _infix_fns[Token::GTE] = &parser::infix_expr;
  _infix_fns[Token::L_PAREN] = &parser::call_expr;
  _infix_fns[Token::L_BRACKET] = &parser::index_expr;

  std::vector<parse_tree::toplevel *> top_level_items;
  parse_tree::toplevel *new_top_level_item;

  while (_parser_okay && _idx < _tokens->size()) {

    /*
       Check for an import statement
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

      auto [item_found, target_item] =
          locate_import(include_directories, import_statement->target);

      if (!item_found) {
        std::cout << "Error : Unable to locate import \""
                  << import_statement->target << "\"" << std::endl;
        delete new_top_level_item;
        _parser_okay = false;
        continue;
      }

      // Lex and parse the file
      std::vector<TD_Pair> imported_tokens = import_file(target_item);

      parser import_parser;
      std::vector<parse_tree::toplevel *> parsed_file = import_parser.parse(
          target_item, include_directories, import_file, imported_tokens);
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

    /*
       Check for a function declaration
       */
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

void parser::prev() { _idx--; }

void parser::advance() { _idx++; }

void parser::die(std::string error) {

  report_error(_filename, _tokens->at(_idx).line, error);
  _parser_okay = false;
}

void parser::expect(Token token, std::string error, size_t ahead) {

  if (_idx + ahead >= _tokens->size()) {
    die(error);
  }

  if (_tokens->at(_idx + ahead).token != token) {
    die(error);
  }
}

TD_Pair parser::peek(size_t ahead) {
  if (!_tokens) {
    return TD_Pair{Token::EOS, {}};
  }
  if (_idx + ahead >= _tokens->size()) {
    return TD_Pair{Token::EOS, {}};
  }
  return _tokens->at(_idx + ahead);
}

parser::precedence parser::peek_precedence() {
  if (precedences.find(peek().token) != precedences.end()) {
    return precedences[peek().token];
  }
  return parser::precedence::LOWEST;
}

parse_tree::toplevel *parser::import_stmt() {

  if (_tokens->at(_idx).token != Token::IMPORT) {
    return nullptr;
  }
  expect(Token::STRING, "Expected string value for given import", 1);
  advance();

  if (_parser_okay) {
    return new parse_tree::import_stmt(_tokens->at(_idx).data);
  } else {
    return nullptr;
  }
}

parse_tree::toplevel *parser::function() {

  // fn some_function
  if (_tokens->at(_idx).token != Token::FN) {
    return nullptr;
  }

  advance();
  expect(Token::IDENTIFIER, "Expected function name following 'fn'");
  std::string function_name = _tokens->at(_idx).data;

  advance();
  std::vector<parse_tree::variable> parameters = function_params();

  expect(Token::ARROW,
         "Expected '->' following function parameters to denote return type");

  advance();
  expect(Token::IDENTIFIER,
         "Expected return type following '->' in function declaration");
  std::string return_type = _tokens->at(_idx).data;

  advance();
  std::vector<parse_tree::element *> element_list = statements();

  if (!_parser_okay) {
    return nullptr;
  }

  parse_tree::function *new_func = new parse_tree::function();

  new_func->name = function_name;
  new_func->return_type = parse_tree::string_to_variable_type(return_type);
  new_func->parameters = parameters;
  new_func->element_list = element_list;

  return new_func;
}

std::vector<parse_tree::variable> parser::function_params() {

  expect(Token::L_PAREN,
         "Expected '(' to mark beginning of function parameters");

  // Empty params
  if (peek().token == Token::R_PAREN) {
    advance(); // Eat ')'
    advance(); // Leave parameter decl
    return {};
  }

  std::vector<parse_tree::variable> parameters;

  bool eat_params = true;
  while (eat_params) {

    advance();
    expect(Token::IDENTIFIER, "Expected variable name for parameter");
    std::string param_name = _tokens->at(_idx).data;

    advance();
    expect(Token::COLON,
           "Expected colon between name:type in parameter definition");

    advance();
    expect(Token::IDENTIFIER, "Expected variable type for parameter");
    std::string param_type = _tokens->at(_idx).data;

    advance();
    uint64_t depth = 0;
    if (_tokens->at(_idx).token == Token::L_BRACKET) {
      depth = std::numeric_limits<uint64_t>::max();
      advance();
      expect(Token::R_BRACKET, "Expected closing bracket for array parameter");
      advance();
    }

    parameters.push_back(parse_tree::variable{
        param_name, parse_tree::string_to_variable_type(param_type), depth});

    if (_tokens->at(_idx).token != Token::COMMA) {
      eat_params = false;
    }
  }

  expect(Token::R_PAREN, "Expected ')' to mark the end of function parameters");
  advance();
  return parameters;
}

std::vector<parse_tree::element *> parser::statements() {

  expect(Token::L_BRACE,
         "Expected '{' to mark the beginning of a statement block");
  advance();

  // Check for empty statement body
  if (_tokens->at(_idx).token == Token::R_BRACE) {
    advance();
    return {};
  }

  std::vector<parse_tree::element *> elements;

  //  Check for statements
  //
  bool check_for_statement = true;
  while (check_for_statement) {

    // Attempt to get a statement - Return on error
    parse_tree::element *new_element = statement();
    if (!_parser_okay) {
      for (auto &el : elements) {
        delete el;
      }
      return {};
    }

    // If a new item was gotten, add it to the list
    if (new_element) {
      elements.push_back(new_element);
    } else {
      check_for_statement = false;
    }
  }

  expect(Token::R_BRACE, "Expected '}' to mark the end of a statement block");
  return elements;
}

parse_tree::element *parser::statement() {
  if (parse_tree::element *item = parser::assignment()) {
    return item;
  }
  if (parse_tree::element *item = parser::if_statement()) {
    return item;
  }
  if (parse_tree::element *item = parser::loop()) {
    return item;
  }
  if (parse_tree::element *item = parser::expression_statement()) {
    return item;
  }
  return nullptr;
}

parse_tree::element *parser::assignment() {

  if (_tokens->at(_idx).token != Token::LET) {
    return nullptr;
  }

  advance();
  expect(Token::IDENTIFIER, "Expected variable name in assignmnet");
  std::string name = _tokens->at(_idx).data;
  size_t line_no = *_tokens->at(_idx).line;

  advance();
  expect(Token::COLON,
         "Expected colon between name:type in varialbe assignment");

  advance();
  expect(Token::IDENTIFIER, "Expected variable type");
  std::string variable_type = _tokens->at(_idx).data;

  /*
     Consume [100][3][3]... [?]
     and calculate the number of items that would represent i.e [10][10] = 100
     */
  advance();
  uint64_t depth = 0;
  if (_tokens->at(_idx).token == Token::L_BRACKET) {
    depth = 1;
    bool consume = true;
    while (consume) {
      advance();
      expect(Token::LITERAL_NUMBER, "Literal number expected");
      depth *= std::stoull(_tokens->at(_idx).data);

      advance();
      expect(Token::R_BRACKET, "Ending bracket expected");
      if (peek().token != Token::L_BRACKET) {
        consume = false;
        advance();
      } else {
        advance(); // Eat the '['
      }
    }
  }

  expect(Token::EQ, "Expected '=' in variable assignment");

  advance();

  parse_tree::expression *tree = new parse_tree::expression();
  parse_tree::expression *exp = expression(parser::precedence::LOWEST);

  advance();
  expect(Token::SEMICOLON, "Expected semicolon at end of variable assignment");

  advance();
  if (_parser_okay) {
    return new parse_tree::assignment(
        line_no,
        {name, parse_tree::string_to_variable_type(variable_type), depth}, exp);
  }

  delete tree;
  return nullptr;
}

parse_tree::expression *parser::expression(parser::precedence precedence) {

  if (_prefix_fns.find(_tokens->at(_idx).token) == _prefix_fns.end()) {
    die("No prefix function for given token");
    return nullptr;
  }

  auto fn = _prefix_fns[_tokens->at(_idx).token];
  parse_tree::expression *left = (this->*fn)();

  while (peek().token != Token::SEMICOLON && precedence < peek_precedence()) {
    if (_infix_fns.find(peek().token) == _infix_fns.end()) {
      return left;
    }
    auto i_fn = _infix_fns[peek().token];
    advance();
    left = (this->*i_fn)(left);
  }

  return left;
}

parse_tree::expression *parser::prefix_expr() {

  auto result = new parse_tree::prefix_expr(_tokens->at(_idx).data, nullptr);

  advance();

  result->right = expression(precedence::PREFIX);
  return result;
}

parse_tree::expression *parser::infix_expr(parse_tree::expression *left) {

  auto result =
      new parse_tree::infix_expr(_tokens->at(_idx).data, left, nullptr);

  precedence p = precedence::LOWEST;
  if (precedences.find(_tokens->at(_idx).token) != precedences.end()) {
    p = precedences[_tokens->at(_idx).token];
  }

  advance();

  result->right = expression(p);
  return result;
}

parse_tree::expression *parser::identifier() {

  // Sanity check
  expect(Token::IDENTIFIER, "Expected identifier in expression");
  return new parse_tree::expression(parse_tree::node_type::ID,
                                    _tokens->at(_idx).data);
}

parse_tree::expression *parser::number() {
  if (_tokens->at(_idx).token == Token::LITERAL_NUMBER) {
    return new parse_tree::expression(parse_tree::node_type::RAW_NUMBER,
                                      _tokens->at(_idx).data);
  } else if (_tokens->at(_idx).token == Token::LITERAL_FLOAT) {
    return new parse_tree::expression(parse_tree::node_type::RAW_FLOAT,
                                      _tokens->at(_idx).data);
  } else {
    die("Expected numerical item");
    return nullptr;
  }
}

parse_tree::expression *parser::str() {

  // Sanity check
  expect(Token::STRING, "Expected string in expression");

  return new parse_tree::expression(parse_tree::node_type::RAW_STRING,
                                    _tokens->at(_idx).data);
}

parse_tree::expression *parser::call_expr(parse_tree::expression *fn) {

  parse_tree::function_call_expr *result = new parse_tree::function_call_expr();

  result->fn = fn;

  if (peek().token == Token::R_PAREN) {
    advance();
    return result;
  }

  result->params = parser::expression_list();

  if (!_parser_okay) {
    delete result;
    return nullptr;
  }

  return result;
}

std::vector<parse_tree::expression *> parser::expression_list() {

  std::vector<parse_tree::expression *> results;

  results.emplace_back(expression(precedence::LOWEST));

  while (peek().token == Token::COMMA) {
    advance();
    advance();
    results.emplace_back(expression(precedence::LOWEST));
  }

  if (!_parser_okay) {
    for (auto &e : results) {
      delete e;
    }
    results.clear();
  }

  return results;
}

parse_tree::expression *parser::grouped_expr() {
  advance();
  parse_tree::expression *expr = expression(precedence::LOWEST);

  if (peek().token != Token::R_PAREN) {
    delete expr;
    return nullptr;
  }
  advance();
  return expr;
}

parse_tree::expression *parser::array() {

  parse_tree::array_literal_expr *arr = new parse_tree::array_literal_expr();

  if (peek().token == Token::R_BRACKET) {
    advance();
    return arr;
  }

  arr->expressions = expression_list();

  if (peek().token != Token::R_BRACKET) {
    delete arr;
    return nullptr;
  }

  return arr;
}

parse_tree::expression *parser::index_expr(parse_tree::expression *arr) {

  parse_tree::array_index_expr *idx = new parse_tree::array_index_expr();
  idx->arr = arr;

  advance();
  idx->index = expression(precedence::LOWEST);

  advance();
  expect(Token::R_BRACKET, "Expected ']' following index into array");

  if (!_parser_okay) {
    delete idx;
    return nullptr;
  }

  return idx;
}

parse_tree::element *parser::if_statement() { return nullptr; }
parse_tree::element *parser::else_if_statement() { return nullptr; }
parse_tree::element *parser::else_statement() { return nullptr; }
parse_tree::element *parser::loop() { return nullptr; }
parse_tree::element *parser::expression_statement() { return nullptr; }

} // namespace compiler
