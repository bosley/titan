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

namespace titan {

analyzer::analyzer(std::vector<instructions::instruction_ptr> &tree)
    : _tree(tree), _current_function(nullptr), _num_errors(0),
      _uid(0), _err("analyzer")
{
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

  uint64_t item_count = 0;

  for (auto &item : _tree) {

    if (_num_errors >= NUM_ERRORS_BEFORE_ABORT) {
      return false;
    }

    item_count++;
    LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: Analyzing item number " << item_count << std::endl;

    item->visit(*this);

    LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: Item number "
               << item_count << " complete" << std::endl;
  }

  return _num_errors == 0;
}

void analyzer::receive(instructions::import &ins)
{
  LOG(WARNING) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: Import statement made its way to analyzer"
               << std::endl;
  _num_errors++;
}

void analyzer::receive(instructions::function &ins)
{
  _current_function = &ins;

  //  Attempt to add the item - if it fails its a duplicate
  //
  if (!_table.add_symbol(_current_function->name, _current_function)) {
    std::string msg = "Duplicate function \"" + _current_function->name + "\"";

    auto item = _table.lookup(_current_function->name);
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
        _num_errors++;
        return;
      }
    }

    report_error(error::analyzer::DUPLICATE_FUNCTION_DEF, _current_function->line,
                 _current_function->col, msg, true, _current_function->file_name);
  }

  //  Create a scope for the current function
  //
  _table.add_scope_and_enter(_current_function->name);

  //  Check parameters
  //
  for (auto &param : _current_function->parameters) {
    if (!_table.add_symbol(param->name, param.get())) {
      report_error(error::analyzer::DUPLICATE_PARAMETER,
                   _current_function->line, _current_function->col, "");
      _table.pop_scope();
      return;
    }
  }

  //  Check function body
  //
  for (auto &instruction : _current_function->instruction_list) {
    instruction->visit(*this);
  }

  //  Leave scope
  //
  _table.pop_scope();
}

void analyzer::receive(instructions::define_user_struct &ins)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: User defined struct not yet implemented in analyzer" << std::endl;
  _num_errors++;
}

void analyzer::receive(instructions::scope_change &ins)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: Scope change to : " << ins.scope << std::endl;
}

void analyzer::receive(instructions::assignment_instruction &ins)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: Assignment Statement" << std::endl;

  // Allow shadowing, report duplicates
  //
  if (!_table.add_symbol(ins.var->name, &ins)) {
    auto existing_item = _table.lookup(ins.var->name).value();
    std::string msg = "Duplicate variable name \"";
    msg += ins.var->name;
    msg += "\". Item first defined on line ";
    msg += std::to_string(existing_item.assignment->line);
    report_error(error::analyzer::DUPLICATE_VARIABLE_DEF, ins.line,
                 0, msg, false);
    return;
  }

  auto expression_result = analyze_expression(ins.expr.get());
  
  if(ins.var->classification == instructions::variable_classification::BUILT_IN) {

    // Cast the thing to the built in type
    // create a vtd for the givent hing and update the next line

    auto bit = reinterpret_cast<instructions::built_in_variable*>(ins.var.get());

    std::string msg;
    if (!can_cast_to_expected({ bit->type, bit->depth }, expression_result, msg)) {
      report_error(error::analyzer::IMPLICIT_CAST_FAIL, ins.line, 0,
                   msg, false);
    }

  } else {

    LOG(FATAL) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: Analyzer does not yet handle given variable type" << std::endl;
    _num_errors++;
    return;
  }
}

void analyzer::receive(instructions::expression_instruction &ins)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: Expression Statement" << std::endl;

  // Assignment expressions will be validated automatically
  //
  analyze_expression(ins.expr.get());
}

void analyzer::receive(instructions::if_instruction &ins)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: If Statement"
             << std::endl;

  std::string scope = "if_instruction_";
  for (auto &seg : ins.segments) {

    std::string current_scope = scope + std::to_string(_uid++);
    _table.add_scope_and_enter(current_scope);

    analyze_expression(seg.expr.get());
    for (auto &el : seg.instruction_list) {
      el->visit(*this);
    }

    _table.pop_scope();
  }
}

void analyzer::receive(instructions::while_instruction &ins)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: While Statement"
             << std::endl;

  std::string scope = "while_instruction_" + std::to_string(_uid++);
  _table.add_scope_and_enter(scope);
  analyze_expression(ins.condition.get());
  for (auto &el : ins.body) {
    el->visit(*this);
  }
  _table.pop_scope();
}

void analyzer::receive(instructions::for_instruction &ins)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: For Statement"
             << std::endl;

  std::string scope = "for_instruction_" + std::to_string(_uid++);
  _table.add_scope_and_enter(scope);
  ins.assign->visit(*this);
  analyze_expression(ins.condition.get());
  analyze_expression(ins.modifier.get());
  for (auto &el : ins.body) {
    el->visit(*this);
  }
  _table.pop_scope();
}

void analyzer::receive(instructions::return_instruction &ins)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: Return Statement"
             << std::endl;

  auto var_type_data = retrieve_type_depth(_current_function->return_data.get());

  //  If the return has a statement ensure it matches the return statement
  //
  if (ins.expr.get()) {
    auto expression_result = analyze_expression(ins.expr.get());
    std::string msg;
    if (!can_cast_to_expected(var_type_data, expression_result,
                              msg)) {
      report_error(error::analyzer::IMPLICIT_CAST_FAIL, ins.line,
                   ins.col, msg, false);
    }
  }
  else {
    if (var_type_data.type != instructions::variable_types::UNDEF) {
      std::string message =
          "Expected expression for return in function with non-nil return type";
      report_error(error::analyzer::RETURN_EXPECTED_EXPRESSION,
                   ins.line, ins.col, message);
    }
  }
}

analyzer::vtd analyzer::retrieve_type_depth(instructions::variable *var)
{
  vtd var_type_data;
  if(_current_function->return_data->classification == instructions::variable_classification::BUILT_IN) {
    auto bit = reinterpret_cast<instructions::built_in_variable*>(var);
    var_type_data = { bit->type, bit->depth };
  } else {
    LOG(FATAL) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: Analyzer does not yet handle given variable type" << std::endl;
    _num_errors++;
  }
  return var_type_data;
}

analyzer::vtd analyzer::analyze_expression(instructions::expression *expr)
{
  if (!expr) {
    _num_errors++;
    LOG(ERROR) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: Null expression passed to analyzer" << std::endl;
    return {instructions::variable_types::INT, 0};
  }

  switch (expr->type) {

  case instructions::node_type::ROOT: {
    _num_errors++;
    LOG(ERROR) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: Root expression passed to analyzer" << std::endl;
    return {instructions::variable_types::INT, 0};
  }

  case instructions::node_type::CALL: {
    // Call validation broken out due to size
    auto potential_type = validate_function_call(expr);
    if (std::nullopt != potential_type) {

      return {potential_type.value().type, potential_type.value().depth};
    }
    break;
  }

  case instructions::node_type::ARRAY_IDX: {
    auto array = reinterpret_cast<instructions::array_index_expr *>(expr);
    auto arr_type = analyze_expression(array->arr.get());
    auto arr_idx_type = analyze_expression(array->index.get());
    if (static_cast<uint64_t>(arr_idx_type.type) <
        static_cast<uint64_t>(instructions::variable_types::FLOAT)) {
      return arr_type;
    }

    // Invalid non-integer type
    report_error(error::analyzer::INVALID_ARRAY_IDX, array->line,
                 array->col,
                 "Given type indexing into array is of a non-integer type");
    break;
  }

  case instructions::node_type::INFIX: {
    auto potential_type = validate_infix(expr);
    if (std::nullopt != potential_type) {
      return {potential_type.value(), 0};
    }
    break;
  }

  case instructions::node_type::PREFIX: {
    auto potential_type = validate_prefix(expr);
    if (std::nullopt != potential_type) {
      return {potential_type.value(), 0};
    }
    break;
  }

  case instructions::node_type::ID: {
    auto suspected_id = _table.lookup(expr->value);

    if (suspected_id == std::nullopt) {
      std::string message = "Unknown variable \"";
      message += expr->value;
      message += "\"";
      report_error(error::analyzer::UNKNOWN_ID, expr->line, expr->col,
                   message);
      break;
    }

    if (suspected_id->type != symbol::variant_type::ASSIGNMENT) {

      if (suspected_id->type == symbol::variant_type::PARAMETER) {
        return retrieve_type_depth(suspected_id.value().parameter_variable);
      }

      std::string message = "Item \"";
      message += expr->value;
      message += "\" is not a variable";
      report_error(error::analyzer::EXPECTED_VARIABLE, expr->line,
                   expr->col, message);
      break;
    }

    return retrieve_type_depth(suspected_id.value().assignment->var.get());

  }

  case instructions::node_type::RAW_FLOAT: {
    return {instructions::variable_types::FLOAT, 0};
  }

  case instructions::node_type::RAW_STRING: {
    return {instructions::variable_types::STRING, 0};
  }

  case instructions::node_type::RAW_NUMBER: {
    auto determined_type = determine_integer_type(expr->value);
    if (std::nullopt == determined_type) {
      std::string message = "Unable to determine integer type from value \"";
      message += expr->value;
      message += "\"";
      report_error(
          error::analyzer::INTERNAL_UNABLE_TO_DETERMINE_INT_VAL,
          expr->line, expr->col, message);
      break;
    }

    // Tag the type and value so we don't have to parse again later
    auto raw = reinterpret_cast<instructions::raw_int_expr *>(expr);
    raw->as = std::get<0>(determined_type.value());
    raw->with_val = std::get<1>(determined_type.value());
    return {raw->as, 0};
  }

  case instructions::node_type::ARRAY: {
    auto arr = reinterpret_cast<instructions::array_literal_expr *>(expr);
    for (auto &e : arr->expressions) {
      analyze_expression(e.get());
    }
    return {instructions::variable_types::ARRAY, arr->expressions.size()};
  }

  } // Switch

  _num_errors++;
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: Error in current expression" << std::endl;
  return {instructions::variable_types::INT, 0};
}

bool analyzer::can_cast_to_expected(analyzer::vtd expected_vtd,
                                    analyzer::vtd actual_vtd,
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

  if (expected == instructions::variable_types::UNDEF ||
      actual == instructions::variable_types::UNDEF) {
    msg = "User defined data casting is not yet supported";
    return false;
  }

  if (actual == instructions::variable_types::STRING &&
      expected != instructions::variable_types::STRING) {
    msg = "Can not safely cast directly from type 'string' to non-string data "
          "type";
    return false;
  }

  if ((expected == instructions::variable_types::UNDEF &&
       actual != instructions::variable_types::UNDEF) ||
      (actual == instructions::variable_types::UNDEF &&
       expected != instructions::variable_types::UNDEF)) {
    msg = "Can not cast with nil value";
    return false;
  }

  msg = "";
  return true;
}

std::optional<analyzer::vtd>
analyzer::validate_function_call(instructions::expression *expr)
{
  auto call = reinterpret_cast<instructions::function_call_expr *>(expr);
  auto suspected_fn = _table.lookup(call->fn->value);

  if (suspected_fn == std::nullopt) {
    std::string message = "Unable to locate item \"" + call->fn->value + "\"";
    report_error(error::analyzer::UNKNOWN_ID, expr->line, expr->col,
                 message);
    return std::nullopt;
  }

  if (suspected_fn->type != symbol::variant_type::FUNCTION) {
    std::string message =
        "Call to non-function type \"" + call->fn->value + "\"";
    report_error(error::analyzer::UNMATCHED_CALL, expr->line,
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
    report_error(error::analyzer::PARAM_SIZE_MISMATCH, call->line,
                 call->col, message);
    return std::nullopt;
  }

  for (size_t i = 0; i < fn->parameters.size(); i++) {

    // Validate the parameter
    auto actual = analyze_expression(call->params[i].get());

    // Ensure that the parameters are convertable
    std::string msg;
    if (!can_cast_to_expected(retrieve_type_depth(fn->parameters[i].get()), actual, msg)) {
      report_error(error::analyzer::PARAM_TYPE_MISMATCH, expr->line,
                   expr->col, "Invalid parameter type(s) passed to function");
      return std::nullopt;
    }
  }

  return retrieve_type_depth(fn->return_data.get());
}

std::optional<instructions::variable_types>
analyzer::validate_prefix(instructions::expression *expr)
{
  auto prefix_expr = reinterpret_cast<instructions::prefix_expr *>(expr);
  return analyze_expression(prefix_expr->right.get()).type;
}

std::optional<instructions::variable_types>
analyzer::validate_infix(instructions::expression *expr)
{
  auto infix_expr = reinterpret_cast<instructions::infix_expr *>(expr);
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
    report_error(error::analyzer::INVALID_EXPRESSION, expr->line,
                 expr->col, "Unable to assign items of mismatched depth");
  }

  if (lhs.type == rhs.type) {
    return rhs.type;
  }

  if (lhs.type == instructions::variable_types::ARRAY ||
      rhs.type == instructions::variable_types::ARRAY) {
    report_error(error::analyzer::INVALID_EXPRESSION, expr->line,
                 expr->col, "Unable to assign mismatched types");
  }

  if (static_cast<uint8_t>(lhs.type) >
      static_cast<uint8_t>(instructions::variable_types::STRING)) {
    report_error(error::analyzer::INVALID_EXPRESSION, expr->line,
                 expr->col, "Unable to assign mismatched types");
    return std::nullopt;
  }

  if (static_cast<uint8_t>(rhs.type) >
      static_cast<uint8_t>(instructions::variable_types::STRING)) {
    report_error(error::analyzer::INVALID_EXPRESSION, expr->line,
                 expr->col, "Unable to assign mismatched types");
    return std::nullopt;
  }

  if (static_cast<uint8_t>(lhs.type) > static_cast<uint8_t>(rhs.type)) {
    return lhs.type;
  }

  return rhs.type;
}

std::optional<std::tuple<instructions::variable_types, long long>>
analyzer::determine_integer_type(const std::string &data)
{
  long long value = 0;
  std::istringstream iss(data);
  iss >> value;

  return {{instructions::variable_types::INT, value}};
}

} // namespace compiler
