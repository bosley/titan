#include "parser.hpp"

#include "error/error_list.hpp"
#include "alert/alert.hpp"
#include "app.hpp"
#include "log/log.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <sstream>
#include <tuple>
#include <unordered_map>

namespace titan {

namespace {

std::unordered_map<Token, parser::precedence> precedences = {
    {Token::EQ, parser::precedence::ASSIGN},
    {Token::EQ_EQ, parser::precedence::EQUALS},
    {Token::ADD_EQ, parser::precedence::EQUALS},
    {Token::SUB_EQ, parser::precedence::EQUALS},
    {Token::DIV_EQ, parser::precedence::EQUALS},
    {Token::MUL_EQ, parser::precedence::EQUALS},
    {Token::MOD_EQ, parser::precedence::EQUALS},
    {Token::POW_EQ, parser::precedence::EQUALS},
    {Token::LSH_EQ, parser::precedence::EQUALS},
    {Token::RSH_EQ, parser::precedence::EQUALS},
    {Token::HAT_EQ, parser::precedence::EQUALS},
    {Token::PIPE_EQ, parser::precedence::EQUALS},
    {Token::TILDE_EQ, parser::precedence::EQUALS},
    {Token::AMPERSAND_EQ, parser::precedence::EQUALS},
    {Token::EXCLAMATION_EQ, parser::precedence::EQUALS},
    {Token::LT, parser::precedence::LESS_GREATER},
    {Token::GT, parser::precedence::LESS_GREATER},
    {Token::LTE, parser::precedence::LESS_GREATER},
    {Token::GTE, parser::precedence::LESS_GREATER},
    {Token::RSH, parser::precedence::SHIFT},
    {Token::LSH, parser::precedence::SHIFT},
    {Token::ADD, parser::precedence::SUM},
    {Token::SUB, parser::precedence::SUM},
    {Token::DIV, parser::precedence::PROD},
    {Token::MUL, parser::precedence::PROD},
    {Token::MOD, parser::precedence::PROD},
    {Token::POW, parser::precedence::POW},
    {Token::AMPERSAND, parser::precedence::BITWISE},
    {Token::HAT, parser::precedence::BITWISE},
    {Token::TILDE, parser::precedence::BITWISE},
    {Token::OR, parser::precedence::LOGICAL},
    {Token::AND, parser::precedence::LOGICAL},
    {Token::PIPE, parser::precedence::LOGICAL},
    {Token::L_PAREN, parser::precedence::CALL},
    {Token::L_BRACKET, parser::precedence::INDEX},
};

TD_Pair error_token = {Token::ERT, {}, 0};
TD_Pair end_of_stream = {Token::EOS, {}, 0};


std::string scope_name_from_target(std::string target)
{
  std::filesystem::path p = target;
  p.replace_extension();
  std::string t { p.u8string()};
  std::replace(t.begin(), t.end(), '\\', '/');
  std::string result;
  for(auto& i : t) {
    if(i == '/') {
      result += "::";
    } else {
      result += i;
    }
  }
  return result;
}


} // namespace

parser::parser(imports &file_imports)
    : _parser_okay(true), _idx(0), _mark(std::numeric_limits<uint64_t>::max()),
      _file_imports(file_imports), _err("parser"), _scope_name("GLOBAL")
{
}

std::vector<instructions::instruction_ptr>
parser::parse(std::string source_name, std::vector<TD_Pair> &tokens)
{
  _parser_okay = true;
  _idx = 0;
  _mark = std::numeric_limits<uint64_t>::max();

  _tokens = tokens;
  _source_name = source_name;

  _prefix_fns[Token::IDENTIFIER] = &parser::identifier;
  _prefix_fns[Token::LITERAL_NUMBER] = &parser::number;
  _prefix_fns[Token::LITERAL_FLOAT] = &parser::number;
  _prefix_fns[Token::STRING] = &parser::str;
  _prefix_fns[Token::EXCLAMATION] = &parser::prefix_expr;
  _prefix_fns[Token::TILDE] = &parser::prefix_expr;
  _prefix_fns[Token::SUB] = &parser::prefix_expr;
  _prefix_fns[Token::L_PAREN] = &parser::grouped_expr;
  _prefix_fns[Token::L_BRACE] = &parser::array;

  _infix_fns[Token::ADD] = &parser::infix_expr;
  _infix_fns[Token::SUB] = &parser::infix_expr;
  _infix_fns[Token::DIV] = &parser::infix_expr;
  _infix_fns[Token::MUL] = &parser::infix_expr;
  _infix_fns[Token::MOD] = &parser::infix_expr;
  _infix_fns[Token::EQ] = &parser::infix_expr;
  _infix_fns[Token::EQ_EQ] = &parser::infix_expr;
  _infix_fns[Token::ADD_EQ] = &parser::infix_expr;
  _infix_fns[Token::SUB_EQ] = &parser::infix_expr;
  _infix_fns[Token::DIV_EQ] = &parser::infix_expr;
  _infix_fns[Token::MUL_EQ] = &parser::infix_expr;
  _infix_fns[Token::MOD_EQ] = &parser::infix_expr;
  _infix_fns[Token::POW_EQ] = &parser::infix_expr;
  _infix_fns[Token::AMPERSAND_EQ] = &parser::infix_expr;
  _infix_fns[Token::HAT_EQ] = &parser::infix_expr;
  _infix_fns[Token::PIPE_EQ] = &parser::infix_expr;
  _infix_fns[Token::TILDE_EQ] = &parser::infix_expr;
  _infix_fns[Token::LSH_EQ] = &parser::infix_expr;
  _infix_fns[Token::RSH_EQ] = &parser::infix_expr;
  _infix_fns[Token::EXCLAMATION_EQ] = &parser::infix_expr;
  _infix_fns[Token::LT] = &parser::infix_expr;
  _infix_fns[Token::LTE] = &parser::infix_expr;
  _infix_fns[Token::GT] = &parser::infix_expr;
  _infix_fns[Token::GTE] = &parser::infix_expr;
  _infix_fns[Token::RSH] = &parser::infix_expr;
  _infix_fns[Token::LSH] = &parser::infix_expr;
  _infix_fns[Token::POW] = &parser::infix_expr;
  _infix_fns[Token::OR] = &parser::infix_expr;
  _infix_fns[Token::AND] = &parser::infix_expr;
  _infix_fns[Token::PIPE] = &parser::infix_expr;
  _infix_fns[Token::HAT] = &parser::infix_expr;
  _infix_fns[Token::AMPERSAND] = &parser::infix_expr;
  _infix_fns[Token::L_PAREN] = &parser::call_expr;
  _infix_fns[Token::L_BRACKET] = &parser::index_expr;

  std::vector<instructions::instruction_ptr> top_level_items;

  while (_parser_okay && _idx < _tokens.size()) {

    /*
       Check for an import statement
       */
    if (auto import_instruction = parser::import()) {

      if (!_parser_okay) {
        break;
      }

      auto [item_found, target_item] =
          locate_import(_file_imports.include_directories, import_instruction->target);

      if (!item_found) {
        std::string message =
            "Unable to locate import target: " + import_instruction->target;
        die(error::parser::UNABLE_TO_LOCATE_IMPORT, message);
        break;
      }

      std::vector<TD_Pair> imported_tokens = _file_imports.import_file(target_item);

      parser import_parser(_file_imports);

      import_parser._scope_name = scope_name_from_target(import_instruction->target);

      auto parsed_file = import_parser.parse(target_item, imported_tokens);

      if (!import_parser.is_okay()) {
        _parser_okay = false;
        break;
      }

      // Change to target scope
      top_level_items.emplace_back(instructions::scope_change_ptr(new instructions::scope_change(
              import_parser._scope_name)));

      // Add it to our top level objects
      top_level_items.insert(top_level_items.end(),
                             std::make_move_iterator(parsed_file.begin()),
                             std::make_move_iterator(parsed_file.end()));

      // Come back to current scope
      top_level_items.emplace_back(instructions::scope_change_ptr(new instructions::scope_change(
              _scope_name)));

      tokens.clear();
    }

    /*
       Check for a function declaration
       */
    else if (auto function_instruction = parser::function()) {

      top_level_items.push_back(std::move(function_instruction));
    }
    else if (auto statement = parser::statement() ) {

      top_level_items.push_back(std::move(statement));
    }
    else {
      die(error::parser::INVALID_TL_ITEM, "");
    };
  }

  _tokens.clear();

  if (!_parser_okay) {
    top_level_items.clear();
  }

  return top_level_items;
}

void parser::report_error(uint64_t error_no, size_t line, size_t col,
                          const std::string msg, bool show_full)
{
   if(_source_name == "repl") {
     show_full = false;
   }

   alert::config cfg;
   cfg.set_basic(_source_name, msg, line, col);
   cfg.set_show_chunk(show_full);
   cfg.set_all_attn(show_full);
  _err.raise(error_no, &cfg);
  _parser_okay = false;
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
    _err.raise(error::parser::INTERNAL_MARK_UNSET);
    _parser_okay = false;
    unset(); // ensure its set to max, not some other num
    return;
  }

  while (_idx != _mark) {
    prev();
  }

  unset();
}

void parser::die(uint64_t error_no, std::string error)
{
  report_error(error_no, current_td_pair().line, current_td_pair().col, error,
               _parser_okay);

  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: " << COLOR(magenta)
             << "Curernt token : " << token_to_str(current_td_pair())
             << COLOR(none) << std::endl;
}

void parser::expect(Token token, std::string error, size_t ahead)
{
  if (peek(ahead).token != token) {
    die(error::parser::UNEXPECTED_TOKEN, error);
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

instructions::import_ptr parser::import()
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
    return instructions::import_ptr(new instructions::import(target, line, col));
  }
  else {
    return nullptr;
  }
}

instructions::instruction_ptr parser::function()
{
  if (current_td_pair().token != Token::FN) {
    return nullptr;
  }

  size_t line = current_td_pair().line;
  size_t col = current_td_pair().col;

  advance();
  expect(Token::IDENTIFIER, "Expected function name following 'fn'");
  std::string function_name = current_td_pair().data;

  advance();
  std::vector<instructions::variable_ptr> parameters = function_params();

  expect(Token::ARROW,
         "Expected '->' following function parameters to denote return type");

  advance();
  expect(Token::IDENTIFIER,
         "Expected return type following '->' in function declaration");
  std::string return_type = current_td_pair().data;

  advance();

  auto [return_depth, segments] = accessor_lit();

  std::vector<instructions::instruction_ptr> instruction_list = statements();

  if (!_parser_okay) {
    return nullptr;
  }

  auto new_func = new instructions::function(line, col);

  new_func->name = function_name;
  new_func->file_name = _source_name;

  new_func->return_data = instructions::variable_ptr(
      new instructions::built_in_variable(
        "return_data",
        instructions::string_to_variable_type(return_type),
        return_depth,
        segments)
      );

  new_func->parameters = std::move(parameters);
  new_func->instruction_list = std::move(instruction_list);

  return instructions::instruction_ptr(new_func);
}

std::vector<instructions::variable_ptr> parser::function_params()
{

  expect(Token::L_PAREN,
         "Expected '(' to mark beginning of function parameters");

  // Empty params
  if (peek().token == Token::R_PAREN) {
    advance(); // Eat ')'
    advance(); // Leave parameter decl
    return {};
  }

  std::vector<instructions::variable_ptr> parameters;

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
    auto param_v_type = instructions::string_to_variable_type(param_type);

    advance();
    auto [depth, segments] = accessor_lit();
    
    parameters.emplace_back(
        instructions::variable_ptr(
          new instructions::built_in_variable(
            param_name,
            param_v_type,
            depth,
            segments
            )
          )
        );

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
std::vector<instructions::instruction_ptr> parser::statements()
{

  expect(Token::L_BRACE,
         "Expected '{' to mark the beginning of a statement block");
  advance();

  // Check for empty statement body
  if (current_td_pair().token == Token::R_BRACE) {
    advance();
    return {};
  }

  std::vector<instructions::instruction_ptr> instructions;

  //  Check for statements
  //
  bool check_for_instruction = true;
  while (check_for_instruction) {

    // Attempt to get a statement - Return on error
    instructions::instruction_ptr new_instruction = statement();
    if (!_parser_okay) {
      return {};
    }

    // If a new item was gotten, add it to the list
    if (new_instruction) {
      instructions.emplace_back(std::move(new_instruction));
    }
    else {
      check_for_instruction = false;
    }
  }

  expect(Token::R_BRACE, "Expected '}' to mark the end of a statement block");
  advance();

  return instructions;
}

std::tuple<uint64_t, std::vector<uint64_t>> parser::accessor_lit()
{
  /*
     Consume [100][3][3]... [?]
     and calculate the number of items that would represent i.e [10][10] = 100
     */
  uint64_t depth = 0;
  std::vector<uint64_t> segments;
  if (current_td_pair().token == Token::L_BRACKET) {
    depth = 1;
    bool consume = true;
    while (consume) {
      advance();
      expect(Token::LITERAL_NUMBER, "Literal number expected");
    
      uint64_t current = 1;
      std::istringstream iss(current_td_pair().data);
      iss >> current;

      depth *= current;
      segments.push_back(current);

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
  return { depth, segments };
}

instructions::instruction_ptr parser::statement()
{
  if (instructions::instruction_ptr item = parser::assignment()) {
    return item;
  }
  if (instructions::instruction_ptr item = parser::if_instruction()) {
    return item;
  }
  if (instructions::instruction_ptr item = parser::while_instruction()) {
    return item;
  }
  if (instructions::instruction_ptr item = parser::for_instruction()) {
    return item;
  }
  if (instructions::instruction_ptr item = parser::return_instruction()) {
    return item;
  }

  // This should be last checked
  if (instructions::instruction_ptr item = parser::expression_instruction()) {
    return item;
  }
  return nullptr;
}

instructions::instruction_ptr parser::assignment()
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
         "Expected colon between name:type in variable assignment");

  advance();
  expect(Token::IDENTIFIER, "Expected variable type");
  auto type_string = current_td_pair().data;
  auto variable_type = instructions::string_to_variable_type(type_string);

  advance();
  auto [depth, segments] = parser::accessor_lit();

  expect(Token::EQ, "Expected '=' in variable assignment");
  advance();

  instructions::expr_ptr exp = expression(parser::precedence::LOWEST);
  advance();
  expect(Token::SEMICOLON, "Expected semicolon at end of variable assignment");

  advance();
  if (_parser_okay) {
    return instructions::instruction_ptr(
        new instructions::assignment_instruction(
          line_no, 
          col,
          instructions::variable_ptr(new instructions::built_in_variable(
            name,
            variable_type,
            depth,
            segments)),
        std::move(exp)));
  }

  return nullptr;
}

instructions::instruction_ptr parser::if_instruction()
{
  if (current_td_pair().token != Token::IF) {
    return nullptr;
  }

  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;

  advance();

  instructions::expr_ptr condition = parser::conditional();

  if (!condition) {
    die(error::parser::EXPECTED_CONDITIONAL, "");
    return nullptr;
  }

  auto if_stmt =
      instructions::if_instruction_ptr(new instructions::if_instruction(line_no, col));

  bool construct_segments = true;

  while (construct_segments && _parser_okay) {

    std::vector<instructions::instruction_ptr> if_body = statements();

    if (!_parser_okay) {
      return nullptr;
    }

    if_stmt->segments.emplace_back(instructions::if_instruction::segment(
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
        condition = instructions::expr_ptr(new instructions::raw_int_expr(
            current_td_pair().line, current_td_pair().col, "1",
            instructions::variable_types::INT, 1));
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

instructions::instruction_ptr parser::while_instruction()
{
  if (current_td_pair().token != Token::WHILE) {
    return nullptr;
  }

  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;

  advance();

  instructions::expr_ptr condition = parser::conditional();

  if (!condition) {
    return nullptr;
  }

  std::vector<instructions::instruction_ptr> body = parser::statements();

  if (!_parser_okay) {
    return nullptr;
  }

  return instructions::instruction_ptr(new instructions::while_instruction(
      line_no, col, std::move(condition), std::move(body)));
}

instructions::instruction_ptr parser::for_instruction()
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
    die(error::parser::EXPECTED_ASSIGNMENT, "first part of for loop");
    return nullptr;
  }

  instructions::expr_ptr conditional =
      parser::expression(parser::precedence::LOWEST);

  advance();

  expect(Token::SEMICOLON, "Expected ';' to mark end of conditional");

  advance();

  instructions::expr_ptr modifier =
      parser::expression(parser::precedence::LOWEST);

  advance();

  expect(Token::R_PAREN, "Expected ')'");

  advance();

  std::vector<instructions::instruction_ptr> body = parser::statements();

  if (!_parser_okay) {
    return nullptr;
  }

  return instructions::instruction_ptr(new instructions::for_instruction(
      line_no, col, std::move(assignment), std::move(conditional),
      std::move(modifier), std::move(body)));
}

instructions::instruction_ptr parser::return_instruction()
{
  if (current_td_pair().token != Token::RETURN) {
    return nullptr;
  }

  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;

  advance();

  instructions::expr_ptr return_expr = nullptr;

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
    return instructions::instruction_ptr(
        new instructions::return_instruction(line_no, col, nullptr));
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

  return instructions::instruction_ptr(
      new instructions::return_instruction(line_no, col, std::move(return_expr)));
}

// Expects expression to exist, if not this will kill the parser
instructions::instruction_ptr parser::expression_instruction()
{
  if (current_td_pair().token == Token::R_BRACE) {
    return nullptr;
  }
  if (current_td_pair().token == Token::SEMICOLON) {
    return nullptr;
  }

  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  instructions::expr_ptr expr = parser::expression(parser::precedence::LOWEST);

  advance();

  expect(Token::SEMICOLON, "Expected semicolon at end of expression");

  advance();

  if (!_parser_okay) {
    return nullptr;
  }

  return instructions::instruction_ptr(
      new instructions::expression_instruction(line_no, col, std::move(expr)));
}

instructions::expr_ptr parser::conditional()
{
  expect(Token::L_PAREN, "Expected '(' to mark beginning of conditional");
  advance();

  instructions::expr_ptr conditional_expression =
      parser::expression(parser::precedence::LOWEST);

  advance();

  expect(Token::R_PAREN, "Expected ')' to mark end of conditional");

  if (!_parser_okay) {
    return nullptr;
  }

  advance();

  return conditional_expression;
}

instructions::expr_ptr parser::expression(parser::precedence precedence)
{

  if (_prefix_fns.find(current_td_pair().token) == _prefix_fns.end()) {
    die(error::parser::INTERNAL_NO_FN_FOR_TOK, "");
    return nullptr;
  }

  auto fn = _prefix_fns[current_td_pair().token];
  instructions::expr_ptr left = (this->*fn)();

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

instructions::expr_ptr parser::prefix_expr()
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  auto result = instructions::prefix_expr_ptr(new instructions::prefix_expr(
      line_no, col, current_td_pair().data, nullptr));
  result->tok_op = current_td_pair().token;
  advance();

  result->right = expression(precedence::PREFIX);
  return result;
}

instructions::expr_ptr parser::infix_expr(instructions::expr_ptr left)
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  auto result = instructions::infix_expr_ptr(new instructions::infix_expr(
      line_no, col, current_td_pair().data, std::move(left), nullptr));
  result->tok_op = current_td_pair().token;

  precedence p = precedence::LOWEST;
  if (precedences.find(current_td_pair().token) != precedences.end()) {
    p = precedences[current_td_pair().token];
  }

  advance();

  result->right = expression(p);
  return result;
}

instructions::expr_ptr parser::identifier()
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  expect(Token::IDENTIFIER, "Expected identifier in expression");
  return instructions::expr_ptr(new instructions::expression(
      line_no, col, instructions::node_type::ID, current_td_pair().data));
}

instructions::expr_ptr parser::number()
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  if (current_td_pair().token == Token::LITERAL_NUMBER) {
    return instructions::expr_ptr(
        new instructions::raw_int_expr(line_no, col, current_td_pair().data));
  }
  else if (current_td_pair().token == Token::LITERAL_FLOAT) {
    return instructions::expr_ptr(new instructions::expression(
        line_no, col, instructions::node_type::RAW_FLOAT,
        current_td_pair().data));
  }
  else {
    die(error::parser::INTERNAL_NON_NUMERIC_REACHED,
        "Expected numerical item");
    return nullptr;
  }
}

instructions::expr_ptr parser::str()
{
  // Sanity check
  expect(Token::STRING, "Expected string in expression");

  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  return instructions::expr_ptr(new instructions::expression(
      line_no, col, instructions::node_type::RAW_STRING, current_td_pair().data));
}

instructions::expr_ptr parser::call_expr(instructions::expr_ptr fn)
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  auto result = instructions::function_call_expr_ptr(
      new instructions::function_call_expr(line_no, col));

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

std::vector<instructions::expr_ptr> parser::expression_list()
{
  std::vector<instructions::expr_ptr> results;

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

instructions::expr_ptr parser::grouped_expr()
{
  advance();
  instructions::expr_ptr expr = expression(precedence::LOWEST);

  advance();
  if (current_td_pair().token != Token::R_PAREN) {
    return nullptr;
  }
  return expr;
}

instructions::expr_ptr parser::array()
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  auto arr = instructions::array_literal_expr_ptr(
      new instructions::array_literal_expr(line_no, col));

  if (peek().token == Token::R_BRACE) {
    advance();
    return arr;
  }

  advance();
  arr->expressions = std::move(expression_list());
  advance();

  expect(Token::R_BRACE, "Expected '}' at the end of array expression");

  if (!_parser_okay) {
    return nullptr;
  }

  return arr;
}

instructions::expr_ptr parser::index_expr(instructions::expr_ptr arr)
{
  size_t line_no = current_td_pair().line;
  size_t col = current_td_pair().col;
  auto idx = instructions::array_index_expr_ptr(
      new instructions::array_index_expr(line_no, col));
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

} // namespace titan
