#ifndef COMPILER_PARSER_HPP
#define COMPILER_PARSER_HPP

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "error/error_manager.hpp"

#include "imports.hpp"
#include "instructions.hpp"
#include "tokens.hpp"

namespace titan {

class parser {
public:
  enum class precedence {
    LOWEST,
    ASSIGN,       // =
    LOGICAL,      // || && 
    BITWISE,      // ^ | &
    EQUALS,       // ==
    LESS_GREATER, // > <
    SHIFT,        // >> << 
    SUM,          // +
    PROD,         // *
    POW,          // **
    PREFIX,       // -a or !a
    CALL,         // call()
    INDEX         // []
  };

  parser(imports &file_imports);

  std::vector<instructions::instruction_ptr>
  parse(std::string source_name,
        std::vector<TD_Pair> &tokens);

  bool is_okay() const { return _parser_okay; }

private:
  typedef instructions::expr_ptr (parser::*prefix_parse_fn)();
  typedef instructions::expr_ptr (parser::*infix_parse_fn)(instructions::expr_ptr);

  bool _parser_okay;
  size_t _idx;
  size_t _mark;
  imports &_file_imports;
  error::manager _err;
  std::vector<TD_Pair> _tokens;
  std::unordered_map<Token, prefix_parse_fn> _prefix_fns;
  std::unordered_map<Token, infix_parse_fn> _infix_fns;
  std::string _source_name;
  std::unordered_map<std::string, std::string> _located_items;
  void report_error(uint64_t error_no, size_t line, size_t col,
                    const std::string error, bool show_full);
  void prev();
  void advance();
  void mark();
  void reset();
  void unset();
  const TD_Pair &current_td_pair() const;
  void die(uint64_t error_no, std::string error);
  void expect(Token token, std::string error, size_t ahead = 0);
  const TD_Pair &peek(size_t ahead = 1) const;
  precedence peek_precedence();
  instructions::instruction_ptr function();
  instructions::import_ptr import();
  std::vector<instructions::variable_ptr> function_params();
  std::vector<instructions::instruction_ptr> statements();
  std::tuple<uint64_t, std::vector<uint64_t>> accessor_lit();
  instructions::instruction_ptr statement();
  instructions::instruction_ptr assignment();
  instructions::instruction_ptr if_instruction();
  instructions::instruction_ptr while_instruction();
  instructions::instruction_ptr for_instruction();
  instructions::instruction_ptr expression_instruction();
  instructions::instruction_ptr return_instruction();
  instructions::expr_ptr conditional();
  instructions::expr_ptr expression(precedence precedence);
  instructions::expr_ptr identifier();
  instructions::expr_ptr number();
  instructions::expr_ptr str();
  instructions::expr_ptr prefix_expr();
  instructions::expr_ptr grouped_expr();
  instructions::expr_ptr array();
  instructions::expr_ptr infix_expr(instructions::expr_ptr left);
  instructions::expr_ptr index_expr(instructions::expr_ptr array);
  instructions::expr_ptr call_expr(instructions::expr_ptr function);
  std::vector<instructions::expr_ptr> expression_list();
  std::tuple<bool, std::string> locate_import(std::vector<std::string> &paths,
                                              std::string &target);
};
} // namespace titan

#endif
