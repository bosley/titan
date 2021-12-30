#include "app.hpp"
#include "alert/alert.hpp"
#include "log/log.hpp"
#include "analyzer.hpp"

namespace compiler {


analyzer::analyzer(std::vector<parse_tree::toplevel_ptr> &tree) : _tree(tree), _current_function(nullptr) {

}

bool analyzer::analyze()
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE
             << "]: Starting semeantic analysis" << std::endl;

  uint64_t item_count = 0;
  for (auto &item : _tree) {

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

  return false;
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
