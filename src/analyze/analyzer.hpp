#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "error/error_manager.hpp"
#include "lang/instructions.hpp"
#include "symbols.hpp"

#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace titan {

class analyzer : private instructions::ins_receiver {
public:
  analyzer(std::vector<instructions::instruction_ptr> &parse_tree);

  bool analyze();

private:
  static constexpr uint8_t NUM_ERRORS_BEFORE_ABORT = 10;

  symbol::table _table;
  std::vector<instructions::instruction_ptr> &_tree;

  instructions::function *_current_function;
  uint8_t _num_errors;
  uint64_t _uid;
  error::manager _err;

  struct vtd {
    instructions::variable_types type;
    uint64_t depth;
  };

  void report_error(uint64_t error_no, size_t line, size_t col,
                    const std::string msg, bool show_col = true,
                    std::string file = "");

  virtual void receive(instructions::import &ins) override;
  virtual void receive(instructions::function &ins) override;
  virtual void receive(instructions::define_user_struct &ins) override;
  virtual void receive(instructions::assignment_instruction &ins) override;
  virtual void receive(instructions::expression_instruction &ins) override;
  virtual void receive(instructions::if_instruction &ins) override;
  virtual void receive(instructions::while_instruction &ins) override;
  virtual void receive(instructions::for_instruction &ins) override;
  virtual void receive(instructions::return_instruction &ins) override;
  virtual void receive(instructions::scope_change &ins) override;

  vtd retrieve_type_depth(instructions::variable *var);

  vtd analyze_expression(instructions::expression *expr);

  bool can_cast_to_expected(vtd expected, vtd actual,
                            std::string &out);

  std::optional<vtd>
  validate_function_call(instructions::expression *expr);

  std::optional<instructions::variable_types>
  validate_prefix(instructions::expression *expr);

  std::optional<instructions::variable_types>
  validate_infix(instructions::expression *expr);

  std::optional<std::tuple<instructions::variable_types, long long>>
  determine_integer_type(const std::string &data);
};

} // namespace compiler

#endif
