#include "app.hpp"
#include "alert/alert.hpp"
#include "log/log.hpp"
#include "analyzer.hpp"

#include <algorithm>

namespace compiler {


analyzer::analyzer(symbol::table &table,
                   std::vector<parse_tree::toplevel_ptr> &tree)
    : _table(table), _tree(tree), _current_function(nullptr), _num_errors(0)
{
  _flags.entry_method_exists = false;
}

void analyzer::report_error(const std::string &file, size_t line, size_t col, const std::string &msg)
{
  alert::config cfg;
  cfg.set_basic(file, msg, line, col);

  bool show_full = _num_errors == 0;

  cfg.set_show_chunk(show_full);
  cfg.set_all_attn(show_full);

  cfg.show_line_num = line != 0;
  cfg.show_col_num = cfg.show_line_num;

  alert::show(alert::level::ERROR, "analyzer", cfg);

  _num_errors++;
}

bool analyzer::analyze()
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: Starting semeantic analysis" << std::endl;

  uint64_t item_count = 0;
  
  // Prescan functions
  for (auto &item : _tree) {

    if(item->type == parse_tree::toplevel::tl_type::FUNCTION) {
      
      auto fn = reinterpret_cast<parse_tree::function *>(item.get());
      
      if(!_table.add_symbol(fn->name, fn)) {
        std::string msg = "Duplicate function \"" + fn->name + "\"";

        auto item = _table.lookup(fn->name);
        if(item != std::nullopt) {

          try {
            auto first_fn = std::get<parse_tree::function*>(item.value());
            msg += " First occurance at (";
            msg += first_fn->file_name;
            msg += ", line : ";
            msg += std::to_string(first_fn->line);
            msg += ")";
          }
          catch (const std::bad_variant_access &ex) 
          {
            LOG(ERROR)
                << TAG(APP_FILE_NAME) << "[" << APP_LINE
                << "] exception while accessing suspected function from symbol table :"
                << ex.what() << std::endl;
          }
        }

        report_error(fn->file_name, fn->line, fn->col, msg);
      }

      if(fn->name == EXPECTED_ENTRY_SV) {
        if(fn->return_type != EXPECTED_ENTRY_RETURN_TYPE) {
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

  if(!_flags.entry_method_exists) {
    _num_errors++;
    report_error("program error", 0, 0, "No entry method 'main' found");
    return false;
  }


  for (auto &item : _tree) {

    if(_num_errors >= NUM_ERRORS_BEFORE_ABORT) {
      return false;
    }

    //
    //  TODO : 
    //          Pre scan all functions to build a list of function names and parameters
    //          this way expressions can be sure if the function exists, and we can validate parameters
    //          - Eventually we will want to also pre scan for user types
    //          - We can use the function pre scan to check for existence and unqiueness of main

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
      
      for(auto &element : _current_function->element_list) {
        element->visit(*this);
      }
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

  //  Convert variable type to an actual type from its identifier value. These values
  //  should be represented in a structure that will be populated and present in the next step
  //  so the types can be re-matched

  //  Check if stmt.var already exists in current scope. Allow shadowing
  //
  //


  
  //  Analyze expression to ensure all variables / functions called and function parameters are solid
  //

  //  Attempt to determine expression type to ensure that it matches the given variable, or at least if 
  //  it can be converted to said variable type. If it converts maybe send a warn alert ?
}

void analyzer::accept(parse_tree::expression_statement &stmt)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: Expression Statement" << std::endl;

  // Check that stmt->expr is valid
}

void analyzer::accept(parse_tree::if_statement &stmt)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: If Statement"
             << std::endl;

  // Iterate over segments 
  //  ensure segment->expr is valid
  //  Iterate over segment->element_list
  //    visit each element
}

void analyzer::accept(parse_tree::while_statement &stmt)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: While Statement"
             << std::endl;

  // validate stmt->condition
  // Iterate over stmt->body 
  //  visit each element
}

void analyzer::accept(parse_tree::for_statement &stmt)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: For Statement"
             << std::endl;

  // visit stmt->assign
  // validate condition
  // validate modifier
  // Iterate over stmt->body
  //  visit each element
}

void analyzer::accept(parse_tree::return_statement &stmt)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "]: Return Statement"
             << std::endl;

  // if stmt->expr is set validate it
  //
  // Check that the return type matches that of, or can be casted-to the 
  // return type of _current_function
}

void analyzer::analyze_expression(parse_tree::expression *expr)
{
  /*
   *    We may want to pass a result type to the method on each call
   *    so we can track / elevate the type of expression recursively
   *
   *    Most likely makes sense to return a tuple to denote 
   *    validity, and resulting type
   * */


  if(!expr) {
    return;
  }

  switch(expr->type) {

  case parse_tree::node_type::ROOT:
  {
    return;
  }

  case parse_tree::node_type::CALL: 
  {
    // Ensure function exists / reachable 
    break;
  }

  case parse_tree::node_type::ARRAY_IDX:
  {
    // Ensure expr->arr is valid (exists - should be an ID)
    // Ensure expr->index evaluates to a RAW_NUMBER as its the
    // accessor
    break;
  }

  case parse_tree::node_type::INFIX:
  {
    // expr->op is a string
    // get type of expr->left
    // get tyoe of expr->right
    // ensure that op is valid for the given types
    break;
  }

  case parse_tree::node_type::PREFIX:
  {
    // expr->op is a string
    // get type of expr->right
    // ensure op is valid for given type
    break;
  }

  case parse_tree::node_type::ID:
  {
    // Ensure ID is reachable
    // Return type of ID 
    break;
  }

  case parse_tree::node_type::RAW_FLOAT:
  {
    // Return type
    break;
  }

  case parse_tree::node_type::RAW_STRING:
  {
    // Return type
    break;
  }

  case parse_tree::node_type::RAW_NUMBER:
  {
    // Return type
    break;
  }

  case parse_tree::node_type::ARRAY:
  {
    // Iterate over expr->expressions
    // If we care (don't know yet) ensure all expressions are of same type
    // If we don't, we only need to ensure they are valid (ids/funcs/etc)
    break;
  }

  } // Switch

}

} // Namespace
