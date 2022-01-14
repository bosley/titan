#include "error_manager.hpp"
#include "error_list.hpp"
#include "app.hpp"

#include <iostream>

namespace error {

manager::manager(std::string reporter) : _num_errors(0), _reporter(reporter) {

  _error_map[error::lexer::TARGET_NOT_FILE]  = "Given item is not a file";
  _error_map[error::lexer::TARGET_CANT_OPEN] = "Can not open file";
  
  _error_map[error::parser::INTERNAL_MARK_UNSET] = "Internal - Mark unset";
  _error_map[error::parser::INTERNAL_NO_FN_FOR_TOK] = "Internal - No function to handle token";
  _error_map[error::parser::INTERNAL_NON_NUMERIC_REACHED] = "Internal - Non numeric passed from lexer";
  _error_map[error::parser::UNABLE_TO_LOCATE_IMPORT] = "Unable to locate import";
  _error_map[error::parser::INVALID_TL_ITEM] = "Invalid top level item";
  _error_map[error::parser::EXPECTED_CONDITIONAL] = "Expected a conditional";
  _error_map[error::parser::EXPECTED_ASSIGNMENT] = "Expeccted an assignment";
  _error_map[error::parser::UNEXPECTED_TOKEN] = "Unexpected token";

  _error_map[error::analyzer::INTERNAL_UNABLE_TO_DETERMINE_INT_VAL] = "Can't determine base type for int";
  _error_map[error::analyzer::DUPLICATE_FUNCTION_DEF] = "Duplicate function name";
  _error_map[error::analyzer::DUPLICATE_VARIABLE_DEF] = "Duplicate variable name";
  _error_map[error::analyzer::ENTRY_NOT_FOUND] = "Program entry not found";
  _error_map[error::analyzer::RETURN_EXPECTED_EXPRESSION] = "Return expects expression for non-nil function";
  _error_map[error::analyzer::UNKNOWN_ID] = "Unknown identifier";
  _error_map[error::analyzer::UNMATCHED_CALL] = "Unmatched call";
  _error_map[error::analyzer::EXPECTED_VARIABLE] = "Expected variable";
  _error_map[error::analyzer::PARAM_SIZE_MISMATCH] = "Parameter length mismatch";
  _error_map[error::analyzer::PARAM_TYPE_MISMATCH] = "Parameter type mismatch";
  _error_map[error::analyzer::INCORRECT_ENTRY_RETURN] = "Incorrect return type for entry funtion";
  _error_map[error::analyzer::INVALID_EXPRESSION] = "Invalid expression";
  _error_map[error::analyzer::IMPLICIT_CAST_FAIL] = "Unable to cast to expected type";
  _error_map[error::analyzer::INVALID_ARRAY_IDX] = "Invalid type for indexing into array";
  _error_map[error::analyzer::DUPLICATE_PARAMETER] = "Duplicate parameter in function definition";
}

void manager::raise(uint16_t error_number, alert::config *cfg)
{
  std::cout << APP_COLOR_RED << "Error : " << std::to_string(error_number) << APP_COLOR_END;
  std::cout << " : " << _error_map[error_number] << std::endl; 
  if(!cfg) {
    return;
  }
  alert::show(alert::level::ERROR, _reporter.c_str(), *cfg);
}

}

