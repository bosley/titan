#include "analyzer.hpp"
#include "alert/alert.hpp"
#include "app.hpp"
#include "error/error_list.hpp"
#include "log/log.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace compiler {

analyzer::analyzer(symbol::table &table,
                   std::vector<parse_tree::toplevel_ptr> &tree)
    : _table(table), _tree(tree), _current_function(nullptr), _num_errors(0),
      _uid(0), _err("analyzer")
{
  _flags.entry_method_exists = false;
}

void analyzer::report_error(uint64_t error_no, size_t line, size_t col,
                            const std::string msg, bool show_col,
                            std::string file)
{
  std::string target_file;
  if (!_current_function) {
    if (!file.empty()) {
      target_file = file;
    }
    else {
      _err.raise(error_no);
    }
  }
  else {
    target_file = _current_function->file_name;
  }

  alert::config cfg;

  cfg.set_basic(target_file, msg, line, col);

  bool show_full = _num_errors == 0;

  cfg.set_show_chunk(show_full);
  cfg.set_all_attn(show_full);

  cfg.show_line_num = line != 0;
  cfg.show_col_num = show_col;

  _err.raise(error_no, &cfg);
  _num_errors++;
}

bool analyzer::analyze()
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: Starting semeantic analysis" << std::endl;

  //  Note:
  //    In this first loop is where we can scan for user types and other
  //    fun things that will be added later. At that point it might be a good
  //    idea to bust the loop up into a series of function calls to make it
  //    cleaner

  // Initial program scan
  //
  for (auto &item : _tree) {

    //  Populate global scope with functions and validate entry
    //
    if (item->type == parse_tree::toplevel::tl_type::FUNCTION) {

      auto fn = reinterpret_cast<parse_tree::function *>(item.get());

      //  Attempt to add the item - if it fails its a duplicate
      //
      if (!_table.add_symbol(fn->name, fn)) {
        std::string msg = "Duplicate function \"" + fn->name + "\"";

        auto item = _table.lookup(fn->name);
        if (item != std::nullopt) {

          auto first_fn = item.value();
          if (first_fn.type == symbol::variant_type::FUNCTION) {
            msg += " First occurance at (";
            msg += first_fn.function->file_name;
            msg += ", line : ";
            msg += std::to_string(first_fn.function->line);
            msg += ")";
          }
          else {
            LOG(ERROR) << TAG(APP_FILE_NAME) << "[" << APP_LINE
                       << "] unexpected type from table during presecan :"
                       << std::endl;
            return false;
          }
        }

        report_error(error::compiler::analyzer::DUPLICATE_FUNCTION_DEF,
                     fn->line, fn->col, msg, true, fn->file_name);
      }

      //  Check if the item is entry and contains correct return type
      //
      if (fn->name == EXPECTED_ENTRY_SV) {
        if (fn->return_data.type != EXPECTED_ENTRY_RETURN_TYPE) {
          std::string msg = "Entry method \"";
          msg += EXPECTED_ENTRY_SV;
          msg += "\" is expected to have return type : ";
          msg += EXPECTED_ENTRY_RETURN_TYPE_SV;
          msg += ".";
          report_error(error::compiler::analyzer::INCORRECT_ENTRY_RETURN,
                       fn->line, 0, msg);
        }

        _flags.entry_method_exists = true;
      }
    }
  }

  //  Ensure the entry method was found
  //
  if (!_flags.entry_method_exists) {
    _num_errors++;
    _err.raise(error::compiler::analyzer::ENTRY_NOT_FOUND);
    return false;
  }

  uint64_t item_count = 0;

  //  Call on all top level elements to visit us for validataion
  //
  for (auto &item : _tree) {

    if (_num_errors >= NUM_ERRORS_BEFORE_ABORT) {
      return false;
    }

    item_count++;
    LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: Analyzing item number " << item_count << std::endl;

    switch (item->type) {

    // If this happens its an error
    case parse_tree::toplevel::tl_type::IMPORT:
      LOG(WARNING) << TAG(APP_FILE_NAME) << "[" << APP_LINE
                   << "]: Import statement made its way to analyzer"
                   << std::endl;
      return false;
      break;

    // Function
    case parse_tree::toplevel::tl_type::FUNCTION: {

      _current_function = reinterpret_cast<parse_tree::function *>(item.get());

      std::string scope = "top_level_scope_" + std::to_string(item_count);

      _table.add_scope_and_enter(scope);

      for (auto &param : _current_function->parameters) {
        if (!_table.add_symbol(param.name, &param)) {
          report_error(error::compiler::analyzer::DUPLICATE_PARAMETER,
                       _current_function->line, _current_function->col, "");
          _table.pop_scope();
          return false;
        }
      }

      for (auto &element : _current_function->element_list) {
        element->visit(*this);
      }
      _table.pop_scope();
      break;
    }
    }

    LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: Item number "
               << item_count << " complete" << std::endl;
  }

  return _num_errors == 0;
}

void analyzer::accept(parse_tree::assignment_statement &stmt)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: Assignment Statement" << std::endl;

  // Allow shadowing, report duplicates
  //
  if (!_table.add_symbol(stmt.var.name, &stmt)) {
    auto existing_item = _table.lookup(stmt.var.name).value();
    std::string msg = "Duplicate variable name \"";
    msg += stmt.var.name;
    msg += "\". Item first defined on line ";
    msg += std::to_string(existing_item.assignment->line);
    report_error(error::compiler::analyzer::DUPLICATE_VARIABLE_DEF, stmt.line,
                 0, msg, false);
    return;
  }

  auto expression_result = analyze_expression(stmt.expr.get());

  std::string msg;
  if (!can_cast_to_expected(stmt.var.type_depth, expression_result, msg)) {
    report_error(error::compiler::analyzer::IMPLICIT_CAST_FAIL, stmt.line, 0,
                 msg, false);
  }
}

void analyzer::accept(parse_tree::expression_statement &stmt)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: Expression Statement" << std::endl;

  // Assignment expressions will be validated automatically
  //
  analyze_expression(stmt.expr.get());
}

void analyzer::accept(parse_tree::if_statement &stmt)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: If Statement"
             << std::endl;

  std::string scope = "if_statement_";
  for (auto &seg : stmt.segments) {

    std::string current_scope = scope + std::to_string(_uid++);
    _table.add_scope_and_enter(current_scope);

    analyze_expression(seg.expr.get());
    for (auto &el : seg.element_list) {
      el->visit(*this);
    }

    _table.pop_scope();
  }
}

void analyzer::accept(parse_tree::while_statement &stmt)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: While Statement"
             << std::endl;

  std::string scope = "while_statement_" + std::to_string(_uid++);
  _table.add_scope_and_enter(scope);
  analyze_expression(stmt.condition.get());
  for (auto &el : stmt.body) {
    el->visit(*this);
  }
  _table.pop_scope();
}

void analyzer::accept(parse_tree::for_statement &stmt)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: For Statement"
             << std::endl;

  std::string scope = "for_statement_" + std::to_string(_uid++);
  _table.add_scope_and_enter(scope);
  stmt.assign->visit(*this);
  analyze_expression(stmt.condition.get());
  analyze_expression(stmt.modifier.get());
  for (auto &el : stmt.body) {
    el->visit(*this);
  }
  _table.pop_scope();
}

void analyzer::accept(parse_tree::return_statement &stmt)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: Return Statement"
             << std::endl;

  //  If the return has a statement ensure it matches the return statement
  //
  if (stmt.expr.get()) {
    auto expression_result = analyze_expression(stmt.expr.get());
    std::string msg;
    if (!can_cast_to_expected(_current_function->return_data, expression_result,
                              msg)) {
      report_error(error::compiler::analyzer::IMPLICIT_CAST_FAIL, stmt.line,
                   stmt.col, msg, false);
    }
  }
  else {

    // If no statement exists then expect NIL

    if (_current_function->return_data.type !=
        parse_tree::variable_types::NIL) {

      std::string message =
          "Expected expression for return in function with non-nil return type";
      report_error(error::compiler::analyzer::RETURN_EXPECTED_EXPRESSION,
                   stmt.line, stmt.col, message);
    }
  }
}

parse_tree::vtd analyzer::analyze_expression(parse_tree::expression *expr)
{
  if (!expr) {
    _num_errors++;
    LOG(ERROR) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: Null expression passed to analyzer" << std::endl;
    return {parse_tree::variable_types::U8, 0};
  }

  switch (expr->type) {

  case parse_tree::node_type::ROOT: {
    _num_errors++;
    LOG(ERROR) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: Root expression passed to analyzer" << std::endl;
    return {parse_tree::variable_types::U8, 0};
  }

  case parse_tree::node_type::CALL: {
    // Call validation broken out due to size
    auto potential_type = validate_function_call(expr);
    if (std::nullopt != potential_type) {

      return {potential_type.value().type, potential_type.value().depth};
    }
    break;
  }

  case parse_tree::node_type::ARRAY_IDX: {
    auto array = reinterpret_cast<parse_tree::array_index_expr *>(expr);
    auto arr_type = analyze_expression(array->arr.get());
    auto arr_idx_type = analyze_expression(array->index.get());
    if (static_cast<uint64_t>(arr_idx_type.type) <
        static_cast<uint64_t>(parse_tree::variable_types::FLOAT)) {
      return arr_type;
    }

    // Invalid non-integer type
    report_error(error::compiler::analyzer::INVALID_ARRAY_IDX, array->line,
                 array->col,
                 "Given type indexing into array is of a non-integer type");
    break;
  }

  case parse_tree::node_type::INFIX: {
    auto potential_type = validate_infix(expr);
    if (std::nullopt != potential_type) {
      return {potential_type.value(), 0};
    }
    break;
  }

  case parse_tree::node_type::PREFIX: {
    auto potential_type = validate_prefix(expr);
    if (std::nullopt != potential_type) {
      return {potential_type.value(), 0};
    }
    break;
  }

  case parse_tree::node_type::ID: {
    auto suspected_id = _table.lookup(expr->value);

    if (suspected_id == std::nullopt) {
      std::string message = "Unknown variable \"";
      message += expr->value;
      message += "\"";
      report_error(error::compiler::analyzer::UNKNOWN_ID, expr->line, expr->col,
                   message);
      break;
    }

    if (suspected_id->type != symbol::variant_type::ASSIGNMENT) {

      if (suspected_id->type == symbol::variant_type::PARAMETER) {
        return suspected_id.value().parameter_variable->type_depth;
      }

      std::string message = "Item \"";
      message += expr->value;
      message += "\" is not a variable";
      report_error(error::compiler::analyzer::EXPECTED_VARIABLE, expr->line,
                   expr->col, message);
      break;
    }

    return suspected_id.value().assignment->var.type_depth;
  }

  case parse_tree::node_type::RAW_FLOAT: {
    return {parse_tree::variable_types::FLOAT, 0};
  }

  case parse_tree::node_type::RAW_STRING: {
    return {parse_tree::variable_types::STRING, 0};
  }

  case parse_tree::node_type::RAW_NUMBER: {
    auto determined_type = determine_integer_type(expr->value);
    if (std::nullopt == determined_type) {
      std::string message = "Unable to determine integer type from value \"";
      message += expr->value;
      message += "\"";
      report_error(
          error::compiler::analyzer::INTERNAL_UNABLE_TO_DETERMINE_INT_VAL,
          expr->line, expr->col, message);
      break;
    }

    // Tag the type and value so we don't have to parse again later
    auto raw = reinterpret_cast<parse_tree::raw_int_expr *>(expr);
    raw->as = std::get<0>(determined_type.value());
    raw->with_val = std::get<1>(determined_type.value());
    return {raw->as, 0};
  }

  case parse_tree::node_type::ARRAY: {
    auto arr = reinterpret_cast<parse_tree::array_literal_expr *>(expr);
    for (auto &e : arr->expressions) {
      analyze_expression(e.get());
    }
    return {parse_tree::variable_types::ARRAY, arr->expressions.size()};
  }

  } // Switch

  _num_errors++;
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: Error in current expression" << std::endl;
  return {parse_tree::variable_types::U8, 0};
}

bool analyzer::can_cast_to_expected(parse_tree::vtd expected_vtd,
                                    parse_tree::vtd actual_vtd,
                                    std::string &msg)
{
  /*

    TODO:
      A more clever way needs to be devised to do this so we can easily drop in
      user stuff too.

  */

  auto expected = expected_vtd.type;
  auto actual = actual_vtd.type;

  // Ensure expression { .... } for array is <= the definition on the lhs
  if (actual_vtd.depth > expected_vtd.depth) {
    msg = "Unable to cast items of mismatched depth";
    return false;
  }

  if (expected == actual) {
    return true;
  }

  // Check if the actual value yielded by some expression can be cast to the
  // expected value

  if (expected == parse_tree::variable_types::USER_DEFINED ||
      actual == parse_tree::variable_types::USER_DEFINED) {
    msg = "User defined data casting is not yet supported";
    return false;
  }

  if (actual == parse_tree::variable_types::STRING &&
      expected != parse_tree::variable_types::STRING) {
    msg = "Can not safely cast directly from type 'string' to non-string data "
          "type";
    return false;
  }

  if ((expected == parse_tree::variable_types::NIL &&
       actual != parse_tree::variable_types::NIL) ||
      (actual == parse_tree::variable_types::NIL &&
       expected != parse_tree::variable_types::NIL)) {
    msg = "Can not cast with nil value";
    return false;
  }

  msg = "";
  return true;
}

std::optional<parse_tree::vtd>
analyzer::validate_function_call(parse_tree::expression *expr)
{
  auto call = reinterpret_cast<parse_tree::function_call_expr *>(expr);
  auto suspected_fn = _table.lookup(call->fn->value);

  if (suspected_fn == std::nullopt) {
    std::string message = "Unable to locate item \"" + call->fn->value + "\"";
    report_error(error::compiler::analyzer::UNKNOWN_ID, expr->line, expr->col,
                 message);
    return std::nullopt;
  }

  if (suspected_fn->type != symbol::variant_type::FUNCTION) {
    std::string message =
        "Call to non-function type \"" + call->fn->value + "\"";
    report_error(error::compiler::analyzer::UNMATCHED_CALL, expr->line,
                 expr->col, message);
    return std::nullopt;
  }

  auto fn = suspected_fn->function;

  if (fn->parameters.size() != call->params.size()) {
    std::string message = "Expected ";
    message += std::to_string(fn->parameters.size());
    message += " parameters to function ";
    message += call->fn->value;
    message += " but received ";
    message += std::to_string(call->params.size());
    message += " parameters.";
    report_error(error::compiler::analyzer::PARAM_SIZE_MISMATCH, call->line,
                 call->col, message);
    return std::nullopt;
  }

  for (size_t i = 0; i < fn->parameters.size(); i++) {

    // Validate the parameter
    auto actual = analyze_expression(call->params[i].get());

    // Ensure that the parameters are convertable
    std::string msg;
    if (!can_cast_to_expected(fn->parameters[i].type_depth, actual, msg)) {
      report_error(error::compiler::analyzer::PARAM_TYPE_MISMATCH, expr->line,
                   expr->col, "Invalid parameter type(s) passed to function");
      return std::nullopt;
    }
  }

  return {fn->return_data};
}

std::optional<parse_tree::variable_types>
analyzer::validate_prefix(parse_tree::expression *expr)
{
  auto prefix_expr = reinterpret_cast<parse_tree::prefix_expr *>(expr);
  return analyze_expression(prefix_expr->right.get()).type;
}

std::optional<parse_tree::variable_types>
analyzer::validate_infix(parse_tree::expression *expr)
{
  auto infix_expr = reinterpret_cast<parse_tree::infix_expr *>(expr);
  auto lhs = analyze_expression(infix_expr->left.get());
  auto rhs = analyze_expression(infix_expr->right.get());

  /*
   *  Check if expression type needs to be modified to allow expression
   *
   *  Precedence:
   *    uints < ints < floats < string
   *
   * */

  if (lhs.depth != rhs.depth) {
    report_error(error::compiler::analyzer::INVALID_EXPRESSION, expr->line,
                 expr->col, "Unable to assign items of mismatched depth");
  }

  if (lhs.type == rhs.type) {
    return rhs.type;
  }

  if (lhs.type == parse_tree::variable_types::ARRAY ||
      rhs.type == parse_tree::variable_types::ARRAY) {
    report_error(error::compiler::analyzer::INVALID_EXPRESSION, expr->line,
                 expr->col, "Unable to assign mismatched types");
  }

  if (static_cast<uint8_t>(lhs.type) >
      static_cast<uint8_t>(parse_tree::variable_types::STRING)) {
    report_error(error::compiler::analyzer::INVALID_EXPRESSION, expr->line,
                 expr->col, "Unable to assign mismatched types");
    return std::nullopt;
  }

  if (static_cast<uint8_t>(rhs.type) >
      static_cast<uint8_t>(parse_tree::variable_types::STRING)) {
    report_error(error::compiler::analyzer::INVALID_EXPRESSION, expr->line,
                 expr->col, "Unable to assign mismatched types");
    return std::nullopt;
  }

  if (static_cast<uint8_t>(lhs.type) > static_cast<uint8_t>(rhs.type)) {
    return lhs.type;
  }

  return rhs.type;
}

std::optional<std::tuple<parse_tree::variable_types, long long>>
analyzer::determine_integer_type(const std::string &data)
{
  long long value = 0;
  std::istringstream iss(data);
  iss >> value;

  if (value >= 0) {
    if (value <= std::numeric_limits<int8_t>::max()) {
      return {{parse_tree::variable_types::I8, value}};
    }
    else if (value <= std::numeric_limits<int16_t>::max()) {
      return {{parse_tree::variable_types::I16, value}};
    }
    else if (value <= std::numeric_limits<int32_t>::max()) {
      return {{parse_tree::variable_types::I32, value}};
    }
    else {
      return {{parse_tree::variable_types::I64, value}};
    }
  }
  else {
    if (value <= std::numeric_limits<uint8_t>::max()) {
      return {{parse_tree::variable_types::U8, value}};
    }
    else if (value <= std::numeric_limits<uint16_t>::max()) {
      return {{parse_tree::variable_types::U16, value}};
    }
    else if (value <= std::numeric_limits<uint32_t>::max()) {
      return {{parse_tree::variable_types::I32, value}};
    }
    else {
      return {{parse_tree::variable_types::U64, value}};
    }
  }
  return std::nullopt;
}

} // namespace compiler
