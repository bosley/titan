#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "parsetree.hpp"
#include "symbols.hpp"

#include <string>
#include <vector>

namespace compiler {

class analyzer : private parse_tree::visitor {
public:
  analyzer(symbol::table &table,
      std::vector<parse_tree::toplevel_ptr> &parse_tree);

  bool analyze();

private:
  static constexpr std::string_view EXPECTED_ENTRY_SV = "main";
  static constexpr parse_tree::variable_types EXPECTED_ENTRY_RETURN_TYPE = parse_tree::variable_types::I8;
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

  parse_tree::function* _current_function;
  uint8_t _num_errors;

  check_flags _flags;

  void report_error(const std::string &file, size_t line, size_t col, const std::string &msg);

  virtual void accept(parse_tree::assignment_statement &stmt) override;
  virtual void accept(parse_tree::expression_statement &stmt) override;
  virtual void accept(parse_tree::if_statement &stmt) override;
  virtual void accept(parse_tree::while_statement &stmt) override;
  virtual void accept(parse_tree::for_statement &stmt) override;
  virtual void accept(parse_tree::return_statement &stmt) override;

  void analyze_expression(parse_tree::expression *expr);
};

}

#endif
