#include "analyzer.hpp"
#include "alert/alert.hpp"
#include "app.hpp"
#include "log/log.hpp"

#include <iostream>

#include <algorithm>

namespace compiler {

analyzer::analyzer(symbol::table &table,
                   std::vector<parse_tree::toplevel_ptr> &tree)
    : _table(table), _tree(tree), _current_function(nullptr), _num_errors(0),
      _uid(0)
{
  _flags.entry_method_exists = false;
}

void analyzer::report_error(const std::string &file, size_t line, size_t col,
                            const std::string &msg, bool show_col)
{
  alert::config cfg;
  cfg.set_basic(file, msg, line, col);

  bool show_full = _num_errors == 0;

  cfg.set_show_chunk(show_full);
  cfg.set_all_attn(show_full);

  cfg.show_line_num = line != 0;
  cfg.show_col_num = show_col;

  alert::show(alert::level::ERROR, "analyzer", cfg);

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

        report_error(fn->file_name, fn->line, fn->col, msg);
      }

      //  Check if the item is entry and contains correct return type
      //
      if (fn->name == EXPECTED_ENTRY_SV) {
        if (fn->return_type != EXPECTED_ENTRY_RETURN_TYPE) {
          std::string msg = "Entry method \"";
          msg += EXPECTED_ENTRY_SV;
          msg += "\" is expected to have return type : ";
          msg += EXPECTED_ENTRY_RETURN_TYPE_SV;
          msg += ".";
          report_error(fn->file_name, fn->line, 0, msg);
        }

        _flags.entry_method_exists = true;
      }
    }
  }

  //  Ensure the entry method was found
  //
  if (!_flags.entry_method_exists) {
    _num_errors++;
    report_error("program error", 0, 0, "No entry method 'main' found", false);
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
    report_error(_current_function->file_name, stmt.line, 0, msg, false);
    return;
  }

  auto expression_result = analyze_expression(stmt.expr.get());

  std::string msg;
  if (!can_cast_to_expected(stmt.var.type, expression_result, msg)) {
    report_error(_current_function->file_name, stmt.line, 0, msg, false);
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
    if (!can_cast_to_expected(_current_function->return_type, expression_result,
                              msg)) {
      report_error(_current_function->file_name, stmt.line, 0, msg,
                   false);
    }
  }
  else {

    // If no statement exists then expect NIL

    if (_current_function->return_type != parse_tree::variable_types::NIL) {

      std::string message =
          "Expected expression for return in function with non-nil return type";
      report_error(_current_function->file_name, stmt.line, 0, message);
    }
  }
}

parse_tree::variable_types
analyzer::analyze_expression(parse_tree::expression *expr)
{
  /*
   *    We may want to pass a result type to the method on each call
   *    so we can track / elevate the type of expression recursively
   *
   *    Most likely makes sense to return a tuple to denote
   *    validity, and resulting type
   * */

  if (!expr) {
    _num_errors++;
    LOG(ERROR) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: Null expression passed to analyzer" << std::endl;
    return parse_tree::variable_types::U8;
  }

  switch (expr->type) {

  case parse_tree::node_type::ROOT: {
    _num_errors++;
    LOG(ERROR) << TAG(APP_FILE_NAME) << "[" << APP_LINE
               << "]: Root expression passed to analyzer" << std::endl;
    return parse_tree::variable_types::U8;
  }

  case parse_tree::node_type::CALL: {

    auto call = reinterpret_cast<parse_tree::function_call_expr*>(expr);

    std::cout << "Call to " << call->fn->value << std::endl;

    auto fn = _table.lookup(call->fn->value);

    if(fn == std::nullopt) {
      // Report error that the function wasn't found
    }

    // ensure that 'call's function parameters
    // match that of 'fn's in every way
    //
    // return the return_type of the fn call

    break;
  }

  case parse_tree::node_type::ARRAY_IDX: {
    // Ensure expr->arr is valid (exists - should be an ID)
    // Ensure expr->index evaluates to a RAW_NUMBER as its the
    // accessor
    //
    //
    // Need to return the tye that the array holds
    break;
  }

  case parse_tree::node_type::INFIX: {
    // expr->op is a string
    // get type of expr->left
    // get tyoe of expr->right
    // ensure that op is valid for the given types
    break;
  }

  case parse_tree::node_type::PREFIX: {
    // expr->op is a string
    // get type of expr->right
    // ensure op is valid for given type
    break;
  }

  case parse_tree::node_type::ID: {
    // Ensure ID is reachable
    // Return type of ID
    break;
  }

  case parse_tree::node_type::RAW_FLOAT: {
    return parse_tree::variable_types::FLOAT;
  }

  case parse_tree::node_type::RAW_STRING: {
    return parse_tree::variable_types::STRING;
  }

  case parse_tree::node_type::RAW_NUMBER: {
    /*
     *
     *    Need to check out the value of the number and determine what it
     *    fits into - OR we could keep i64 and let the next stage cast it.
     *
     * */
    return parse_tree::variable_types::I64;
  }

  case parse_tree::node_type::ARRAY: {
    // Iterate over expr->expressions
    // If we care (don't know yet) ensure all expressions are of same type
    // If we don't, we only need to ensure they are valid (ids/funcs/etc)
    return parse_tree::variable_types::ARRAY;
  }

  } // Switch

  _num_errors++;
  LOG(ERROR) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: Unhandled node type in analyze_expression" << std::endl;
  return parse_tree::variable_types::U8;
}

bool analyzer::can_cast_to_expected(parse_tree::variable_types expected,
                                    parse_tree::variable_types actual,
                                    std::string &msg)
{
  /*
      
    TODO:
      A more clever way needs to be devised to do this so we can easily drop in
      user stuff too. 

  */
  if(expected == actual) {
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
      actual   != parse_tree::variable_types::NIL) ||
      (actual   == parse_tree::variable_types::NIL &&
      expected != parse_tree::variable_types::NIL)) {
    msg = "Can not cast with nil value";
    return false;
  }

  msg = "";
  return true;
}

} // namespace compiler
