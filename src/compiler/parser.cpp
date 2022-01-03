#include "parser.hpp"

#include "alert/alert.hpp"
#include "app.hpp"
#include "log/log.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <limits>
#include <tuple>
#include <unordered_map>

namespace compiler {

namespace {

std::unordered_map<Token, parser::precedence> precedences = {
    {Token::EQ, parser::precedence::ASSIGN},
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

TD_Pair error_token = {Token::ERT, {}, 0};
TD_Pair end_of_stream = {Token::EOS, {}, 0};

static void report_error(const std::string &filename, size_t line, size_t col,
                         const std::string error, bool show_full)
{
  alert::config cfg;

  cfg.set_basic(filename, error, line, col);
  cfg.set_show_chunk(show_full);
  cfg.set_all_attn(show_full);

  alert::show(alert::level::ERROR, "parser", cfg);
}
} // namespace

parser::parser(imports &file_imports)
    : _parser_okay(true), _idx(0), _mark(std::numeric_limits<uint64_t>::max()),
      _file_imports(file_imports)
{
}

std::vector<parse_tree::toplevel_ptr>
parser::parse(std::string filename,
              std::vector<std::string> &include_directories,
              std::function<std::vector<TD_Pair>(std::string)> import_file,
              std::vector<TD_Pair> &tokens)
{
  _tokens = std::move(tokens);
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
  _infix_fns[Token::EQ] = &parser::infix_expr;
  _infix_fns[Token::EQ_EQ] = &parser::infix_expr;
  _infix_fns[Token::EXCLAMATION_EQ] = &parser::infix_expr;
  _infix_fns[Token::LT] = &parser::infix_expr;
  _infix_fns[Token::LTE] = &parser::infix_expr;
  _infix_fns[Token::GT] = &parser::infix_expr;
  _infix_fns[Token::GTE] = &parser::infix_expr;
  _infix_fns[Token::L_PAREN] = &parser::call_expr;
  _infix_fns[Token::L_BRACKET] = &parser::index_expr;

  std::vector<parse_tree::toplevel_ptr> top_level_items;

  while (_parser_okay && _idx < _tokens.size()) {

    /*
       Check for an import statement
       */
    if (auto import_statement = parser::import()) {

      if (!_parser_okay) {
        break;
      }

      auto [item_found, target_item] =
          locate_import(include_directories, import_statement->target);

      if (!item_found) {
        report_error(_filename, current_td_pair().line, current_td_pair().col,
                     "Unable to locate import target: " +
                         import_statement->target,
                     _parser_okay);
        _parser_okay = false;
        break;
      }

      std::vector<TD_Pair> imported_tokens = import_file(target_item);

      parser import_parser(_file_imports);

      auto parsed_file = import_parser.parse(target_item, include_directories,
                                             import_file, imported_tokens);

      if (!import_parser.is_okay()) {
        _parser_okay = false;
        break;
      }

      // Add it to our top level objects
      top_level_items.insert(top_level_items.end(),
                             std::make_move_iterator(parsed_file.begin()),
                             std::make_move_iterator(parsed_file.end()));
      tokens.clear();
    }

    /*
       Check for a function declaration
       */
    else if (auto function_statement = parser::function()) {

      top_level_items.push_back(std::move(function_statement));
    }
    else {
      die("Invalid top level item. Expected 'import' or 'fn'");
    };
  }

  _tokens.clear();

  if (!_parser_okay) {
    top_level_items.clear();
  }

  return top_level_items;
}

void parser::prev() { _idx--; }

void parser::advance() { _idx++; }

void parser::mark() { _mark = _idx; }

void parser::unset() { _mark = std::numeric_limits<uint64_t>::max(); }

const TD_Pair &parser::current_td_pair() const
{
  if (_idx >= _tokens.size()) {
    LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: End of token stream" << std::endl;
    return error_token;
  }

  return _tokens.at(_idx);
}

void parser::reset()
{

  if (_mark > _idx) {
    die("Internal error - Attempting to reset with unset _mark");
    unset(); // ensure its set to max, not some other num
    return;
  }

  while (_idx != _mark) {
    prev();
  }

  unset();
}

void parser::die(std::string error)
{
  report_error(_filename, current_td_pair().line, current_td_pair().col, error,
               _parser_okay);
  _parser_okay = false;

  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: " << COLOR(magenta)
             << "Curernt token : " << token_to_str(current_td_pair())
             << COLOR(none) << std::endl;
}

void parser::expect(Token token, std::string error, size_t ahead)
{
  if (peek(ahead).token != token) {
    die(error);
  }
}

const TD_Pair &parser::peek(size_t ahead) const
{
  if (_idx + ahead >= _tokens.size()) {
    LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: End of token stream" << std::endl;
    return end_of_stream;
  }
  return _tokens.at(_idx + ahead);
}

parser::precedence parser::peek_precedence()
{
  if (precedences.find(peek().token) != precedences.end()) {
    return precedences[peek().token];
  }
  return parser::precedence::LOWEST;
}

parse_tree::import_ptr parser::import()
{
  if (current_td_pair().token != Token::IMPORT) {
    return nullptr;
  }

  size_t line = current_td_pair().line;
  size_t col = current_td_pair().col;

  expect(Token::STRING, "Expected string value for given import", 1);
  advance();

  if (_parser_okay) {
    auto target = current_td_pair().data;
    advance();
    return parse_tree::import_ptr(new parse_tree::import(target, line, col));
  }
  else {
    return nullptr;
  }
}

parse_tree::function_ptr parser::function()
{

  // fn some_function
  if (current_td_pair().token != Token::FN) {
    return nullptr;
  }

  size_t line = current_td_pair().line;
  size_t col = current_td_pair().col;

  advance();
  expect(Token::IDENTIFIER, "Expected function name following 'fn'");
  std::string function_name = current_td_pair().data;

  advance();
  std::vector<parse_tree::variable> parameters = function_params();

  expect(Token::ARROW,
         "Expected '->' following function parameters to denote return type");

  advance();
  expect(Token::IDENTIFIER,
         "Expected return type following '->' in function declaration");
  std::string return_type = current_td_pair().data;

  advance();
  std::vector<parse_tree::element_ptr> element_list = statements();

  if (!_parser_okay) {
    return nullptr;
  }

  auto new_func = parse_tree::function_ptr(new parse_tree::function(line, col));

  new_func->name = function_name;
  new_func->file_name = _filename;
  new_func->return_type = parse_tree::string_to_variable_type(return_type);
  new_func->parameters = std::move(parameters);
  new_func->element_list = std::move(element_list);

  return new_func;
}

std::vector<parse_tree::variable> parser::function_params()
{

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
    std::string param_name = current_td_pair().data;

    advance();
    expect(Token::COLON,
           "Expected colon between name:type in parameter definition");

    advance();
    expect(Token::IDENTIFIER, "Expected variable type for parameter");
    auto param_type = current_td_pair().data;
    auto param_v_type = parse_tree::string_to_variable_type(param_type);

    advance();
    uint64_t depth = 0;
    if (current_td_pair().token == Token::L_BRACKET) {
      depth = std::numeric_limits<uint64_t>::max();
      advance();
      expect(Token::R_BRACKET, "Expected closing bracket for array parameter");
      advance();
    }

    parameters.push_back(
        parse_tree::variable{param_name, param_v_type, param_type, depth});

    if (current_td_pair().token != Token::COMMA) {
      eat_params = false;
    }
  }

  expect(Token::R_PAREN, "Expected ')' to mark the end of function parameters");
  advance();
  return parameters;
}

//  Eat all of { s, s+1, s+2 ... s+n }
//  including '{' and '}'
//
std::vector<parse_tree::element_ptr> parser::statements()
{

  expect(Token::L_BRACE,
         "Expected '{' to mark the beginning of a statement block");
  advance();

  // Check for empty statement body
  if (current_td_pair().token == Token::R_BRACE) {
    advance();
    return {};
  }

  std::vector<parse_tree::element_ptr> elements;

  //  Check for statements
  //
  bool check_for_statement = true;
  while (check_for_statement) {

    // Attempt to get a statement - Return on error
    parse_tree::element_ptr new_element = statement();
    if (!_parser_okay) {
      return {};
    }

    // If a new item was gotten, add it to the list
    if (new_element) {
      elements.emplace_back(std::move(new_element));
    }
    else {
      check_for_statement = false;
    }
  }

  expect(Token::R_BRACE, "Expected '}' to mark the end of a statement block");
  advance();

  return elements;
}

uint64_t parser::accessor_lit()
{

  /*
     Consume [100][3][3]... [?]
     and calculate the number of items that would represent i.e [10][10] = 100
     */
  uint64_t depth = 0;
  if (current_td_pair().token == Token::L_BRACKET) {
    depth = 1;
    bool consume = true;
    while (consume) {
      advance();
      expect(Token::LITERAL_NUMBER, "Literal number expected");
      depth *= std::stoull(current_td_pair().data);

      advance();
      expect(Token::R_BRACKET, "Ending bracket expected");
      if (peek().token != Token::L_BRACKET) {
        consume = false;
        advance();
      }
      else {
        advance(); // Eat the '['
      }
    }
  }
  return depth;
}

parse_tree::element_ptr parser::statement()
{
  if (parse_tree::element_ptr item = parser::assignment()) {
    return item;
  }
  if (parse_tree::element_ptr item = parser::if_statement()) {
    return item;
  }
  if (parse_tree::element_ptr item = parser::while_statement()) {
    return item;
  }
  if (parse_tree::element_ptr item = parser::for_statement()) {
    return item;
  }
  if (parse_tree::element_ptr item = parser::return_statement()) {
    return item;
  }

  // This should be last checked
  if (parse_tree::element_ptr item = parser::expression_statement()) {
    return item;
  }
  return nullptr;
}

parse_tree::element_ptr parser::assignment()
{
  if (current_td_pair().token != Token::LET) {
    return nullptr;
  }

  advance();
  expect(Token::IDENTIFIER, "Expected variable name in assignmnet");
  std::string name = current_td_pair().data;
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;

  advance();
  expect(Token::COLON,
         "Expected colon between name:type in variabLe assignment");

  advance();
  expect(Token::IDENTIFIER, "Expected variable type");
  auto type_string = current_td_pair().data;
  auto variable_type = parse_tree::string_to_variable_type(type_string);

  advance();
  uint64_t depth = parser::accessor_lit();

  expect(Token::EQ, "Expected '=' in variable assignment");
  advance();

  parse_tree::expr_ptr exp = expression(parser::precedence::LOWEST);
  advance();
  expect(Token::SEMICOLON, "Expected semicolon at end of variable assignment");

  advance();
  if (_parser_okay) {
    return parse_tree::element_ptr(new parse_tree::assignment_statement(
        line_no, col, {name, variable_type, type_string, depth}, std::move(exp)));
  }

  return nullptr;
}

parse_tree::element_ptr parser::if_statement()
{
  if (current_td_pair().token != Token::IF) {
    return nullptr;
  }

  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;

  advance();

  parse_tree::expr_ptr condition = parser::conditional();

  if (!condition) {
    die("Expected conditional for given if statement");
    return nullptr;
  }

  auto if_stmt =
      parse_tree::if_statement_ptr(new parse_tree::if_statement(line_no, col));

  bool construct_segments = true;

  while (construct_segments && _parser_okay) {

    std::vector<parse_tree::element_ptr> if_body = statements();

    if (!_parser_okay) {
      return nullptr;
    }

    if_stmt->segments.emplace_back(parse_tree::if_statement::segment(
        std::move(condition), std::move(if_body)));

    if (current_td_pair().token == Token::ELSE) {

      advance();

      // Else-if Condition
      if (current_td_pair().token == Token::IF) {

        advance();

        condition = parser::conditional();

        if (!condition) {
          _parser_okay = false;
        }
      }
      else {

        // TRUE for last else statement
        condition = parse_tree::expr_ptr(
            new parse_tree::expression(parse_tree::node_type::RAW_NUMBER, "1"));
      }
    }
    else {
      // No continuing segments
      construct_segments = false;
    }
  }

  if (!_parser_okay) {
    return nullptr;
  }

  return if_stmt;
}

parse_tree::element_ptr parser::while_statement()
{
  if (current_td_pair().token != Token::WHILE) {
    return nullptr;
  }

  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;

  advance();

  parse_tree::expr_ptr condition = parser::conditional();

  if (!condition) {
    return nullptr;
  }

  std::vector<parse_tree::element_ptr> body = parser::statements();

  if (!_parser_okay) {
    return nullptr;
  }

  return parse_tree::element_ptr(new parse_tree::while_statement(
      line_no, col, std::move(condition), std::move(body)));
}

parse_tree::element_ptr parser::for_statement()
{
  if (current_td_pair().token != Token::FOR) {
    return nullptr;
  }

  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;

  advance();

  expect(Token::L_PAREN, "Expected '('");

  advance();

  auto assignment = parser::assignment();

  if (!assignment) {
    die("Expected assignment as first part of for loop");
    return nullptr;
  }

  parse_tree::expr_ptr conditional =
      parser::expression(parser::precedence::LOWEST);

  advance();

  expect(Token::SEMICOLON, "Expected ';' to mark end of conditional");

  advance();

  parse_tree::expr_ptr modifier =
      parser::expression(parser::precedence::LOWEST);

  advance();

  expect(Token::R_PAREN, "Expected ')'");

  advance();

  std::vector<parse_tree::element_ptr> body = parser::statements();

  if (!_parser_okay) {
    return nullptr;
  }

  return parse_tree::element_ptr(new parse_tree::for_statement(
      line_no, col, std::move(assignment), std::move(conditional),
      std::move(modifier), std::move(body)));
}

parse_tree::element_ptr parser::return_statement()
{
  if (current_td_pair().token != Token::RETURN) {
    return nullptr;
  }

  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;

  advance();

  parse_tree::expr_ptr return_expr = nullptr;

  // Optional '(' and ')' around statement?
  if (current_td_pair().token == Token::L_PAREN) {
    advance();
    return_expr = parser::expression(parser::precedence::LOWEST);
    advance();
    expect(Token::R_PAREN, "Expected ')' following return expression");
    advance();
  }
  else if (current_td_pair().token == Token::SEMICOLON) {
    advance();
    return parse_tree::element_ptr(
        new parse_tree::return_statement(line_no, col, nullptr));
  }
  else {
    return_expr = parser::expression(parser::precedence::LOWEST);
    advance();
  }
  expect(Token::SEMICOLON, "Expected semicolon at end of return statement");
  advance();

  if (!_parser_okay) {
    return nullptr;
  }

  return parse_tree::element_ptr(
      new parse_tree::return_statement(line_no, col, std::move(return_expr)));
}

// Expects expression to exist, if not this will kill the parser
parse_tree::element_ptr parser::expression_statement()
{
  if (current_td_pair().token == Token::R_BRACE) {
    return nullptr;
  }
  if (current_td_pair().token == Token::SEMICOLON) {
    return nullptr;
  }

  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  parse_tree::expr_ptr expr = parser::expression(parser::precedence::LOWEST);

  advance();

  expect(Token::SEMICOLON, "Expected semicolon at end of expression");

  advance();

  if (!_parser_okay) {
    return nullptr;
  }

  return parse_tree::element_ptr(
      new parse_tree::expression_statement(line_no, col, std::move(expr)));
}

parse_tree::expr_ptr parser::conditional()
{
  expect(Token::L_PAREN, "Expected '(' to mark beginning of conditional");
  advance();

  parse_tree::expr_ptr conditional_expression =
      parser::expression(parser::precedence::LOWEST);

  advance();

  expect(Token::R_PAREN, "Expected ')' to mark end of conditional");

  if (!_parser_okay) {
    return nullptr;
  }

  advance();

  return conditional_expression;
}

parse_tree::expr_ptr parser::expression(parser::precedence precedence)
{

  if (_prefix_fns.find(current_td_pair().token) == _prefix_fns.end()) {
    die("No prefix function for given token");
    return nullptr;
  }

  auto fn = _prefix_fns[current_td_pair().token];
  parse_tree::expr_ptr left = (this->*fn)();

  while (peek().token != Token::SEMICOLON && peek().token != Token::R_BRACE &&
         precedence < peek_precedence()) {
    if (_infix_fns.find(peek().token) == _infix_fns.end()) {
      return left;
    }
    auto i_fn = _infix_fns[peek().token];
    advance();

    left = (this->*i_fn)(std::move(left));
  }

  return left;
}

parse_tree::expr_ptr parser::prefix_expr()
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  auto result = parse_tree::prefix_expr_ptr(
      new parse_tree::prefix_expr(line_no, col, current_td_pair().data, nullptr));

  advance();

  result->right = expression(precedence::PREFIX);
  return result;
}

parse_tree::expr_ptr parser::infix_expr(parse_tree::expr_ptr left)
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  auto result = parse_tree::infix_expr_ptr(new parse_tree::infix_expr(
      line_no, col, current_td_pair().data, std::move(left), nullptr));

  precedence p = precedence::LOWEST;
  if (precedences.find(current_td_pair().token) != precedences.end()) {
    p = precedences[current_td_pair().token];
  }

  advance();

  result->right = expression(p);
  return result;
}

parse_tree::expr_ptr parser::identifier()
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  expect(Token::IDENTIFIER, "Expected identifier in expression");
  return parse_tree::expr_ptr(new parse_tree::expression( line_no, col,
      parse_tree::node_type::ID, current_td_pair().data));
}

parse_tree::expr_ptr parser::number()
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  if (current_td_pair().token == Token::LITERAL_NUMBER) {
    return parse_tree::expr_ptr(new parse_tree::expression(line_no, col,
        parse_tree::node_type::RAW_NUMBER, current_td_pair().data));
  }
  else if (current_td_pair().token == Token::LITERAL_FLOAT) {
    return parse_tree::expr_ptr(new parse_tree::expression(line_no, col,
        parse_tree::node_type::RAW_FLOAT, current_td_pair().data));
  }
  else {
    die("Expected numerical item");
    return nullptr;
  }
}

parse_tree::expr_ptr parser::str()
{
  // Sanity check
  expect(Token::STRING, "Expected string in expression");

  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  return parse_tree::expr_ptr(new parse_tree::expression(line_no, col,
      parse_tree::node_type::RAW_STRING, current_td_pair().data));
}

parse_tree::expr_ptr parser::call_expr(parse_tree::expr_ptr fn)
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  auto result =
      parse_tree::function_call_expr_ptr(new parse_tree::function_call_expr(line_no, col));

  result->fn = std::move(fn);

  if (peek().token == Token::R_PAREN) {
    advance();
    return result;
  }

  advance();

  result->params = parser::expression_list();

  advance();

  if (!_parser_okay) {
    return nullptr;
  }

  return result;
}

std::vector<parse_tree::expr_ptr> parser::expression_list()
{
  std::vector<parse_tree::expr_ptr> results;

  results.emplace_back(expression(precedence::LOWEST));

  while (peek().token == Token::COMMA) {
    advance();
    advance();
    results.emplace_back(expression(precedence::LOWEST));
  }

  if (!_parser_okay) {
    results.clear();
  }

  return results;
}

parse_tree::expr_ptr parser::grouped_expr()
{
  advance();
  parse_tree::expr_ptr expr = expression(precedence::LOWEST);

  advance();
  if (current_td_pair().token != Token::R_PAREN) {
    return nullptr;
  }
  return expr;
}

parse_tree::expr_ptr parser::array()
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  auto arr =
      parse_tree::array_literal_expr_ptr(new parse_tree::array_literal_expr(line_no, col));

  if (peek().token == Token::R_BRACKET) {
    advance();
    return arr;
  }

  arr->expressions = expression_list();

  if (peek().token != Token::R_BRACKET) {
    return nullptr;
  }

  return arr;
}

parse_tree::expr_ptr parser::index_expr(parse_tree::expr_ptr arr)
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  auto idx =
      parse_tree::array_index_expr_ptr(new parse_tree::array_index_expr(line_no, col));
  idx->arr = std::move(arr);

  advance();
  idx->index = expression(precedence::LOWEST);

  advance();
  expect(Token::R_BRACKET, "Expected ']' following index into array");

  if (!_parser_okay) {
    return nullptr;
  }

  return idx;
}

/* Finds an import */
std::tuple<bool, std::string>
parser::locate_import(std::vector<std::string> &paths, std::string &target)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: " << target
             << std::endl;
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: " << paths.size()
             << " include directories" << std::endl;

  // Check the local directory first
  std::filesystem::path item_as_local = std::filesystem::current_path();
  item_as_local /= target;
  if (std::filesystem::is_regular_file(item_as_local)) {
    return {true, item_as_local};
  }

  // If its not in the same directory as the file importing it, check the
  // already located items
  auto [found, imported_target] = _file_imports.get_target_path(target);
  if (found) {
    return {true, imported_target};
  }

  // If it still isn't found, we need to iterate all include dirs and search for
  // it
  for (auto &dir : paths) {
    std::filesystem::path item_path = dir;
    item_path /= target;

    // If we find the item store it for later before handing off
    if (std::filesystem::is_regular_file(item_path)) {
      _file_imports.store_target_path(target, item_path);
      return {true, item_path};
    }
  }

  return {false, {}};
}

} // namespace compiler
