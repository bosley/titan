#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "error/error_manager.hpp"
#include "parsetree.hpp"
#include "symbols.hpp"

#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace compiler {

class analyzer : private parse_tree::visitor {
public:
  analyzer(symbol::table &table,
           std::vector<parse_tree::toplevel_ptr> &parse_tree);

  bool analyze();

private:
  static constexpr std::string_view EXPECTED_ENTRY_SV = "main";
  static constexpr parse_tree::variable_types EXPECTED_ENTRY_RETURN_TYPE =
      parse_tree::variable_types::I8;
  static constexpr std::string_view EXPECTED_ENTRY_RETURN_TYPE_SV = "i8";
  static constexpr uint8_t NUM_ERRORS_BEFORE_ABORT = 10;

  // Flags that need to be true at the end of analyzing
  // the parse trees
  //
  struct check_flags {

    bool entry_method_exists;
  };

  symbol::table &_table;
  std::vector<parse_tree::toplevel_ptr> &_tree;

  parse_tree::function *_current_function;
  uint8_t _num_errors;
  uint64_t _uid;
  error::manager _err;
  check_flags _flags;

  void report_error(uint64_t error_no, size_t line, size_t col,
                    const std::string msg, bool show_col = true,
                    std::string file = "");

  virtual void accept(parse_tree::assignment_statement &stmt) override;
  virtual void accept(parse_tree::expression_statement &stmt) override;
  virtual void accept(parse_tree::if_statement &stmt) override;
  virtual void accept(parse_tree::while_statement &stmt) override;
  virtual void accept(parse_tree::for_statement &stmt) override;
  virtual void accept(parse_tree::return_statement &stmt) override;

  parse_tree::vtd analyze_expression(parse_tree::expression *expr);

  bool can_cast_to_expected(parse_tree::vtd expected, parse_tree::vtd actual,
                            std::string &out);

  std::optional<parse_tree::vtd>
  validate_function_call(parse_tree::expression *expr);

  std::optional<parse_tree::variable_types>
  validate_prefix(parse_tree::expression *expr);

  std::optional<parse_tree::variable_types>
  validate_infix(parse_tree::expression *expr);

  std::optional<std::tuple<parse_tree::variable_types, long long>>
  determine_integer_type(const std::string &data);
};

} // namespace compiler

#endif