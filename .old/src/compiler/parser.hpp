#ifndef COMPILER_PARSER_HPP
#define COMPILER_PARSER_HPP

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "error/error_manager.hpp"

#include "common.hpp"
#include "imports.hpp"
#include "parsetree.hpp"
#include "tokens.hpp"

namespace compiler {

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

  std::vector<parse_tree::toplevel_ptr>
  parse(std::string filename, std::vector<std::string> &include_directories,
        std::function<std::vector<TD_Pair>(std::string)> import_function,
        std::vector<TD_Pair> &tokens);

  bool is_okay() const { return _parser_okay; }

private:
  typedef parse_tree::expr_ptr (parser::*prefix_parse_fn)();
  typedef parse_tree::expr_ptr (parser::*infix_parse_fn)(parse_tree::expr_ptr);

  bool _parser_okay;
  size_t _idx;
  size_t _mark;
  imports &_file_imports;
  error::manager _err;
  std::vector<TD_Pair> _tokens;
  std::unordered_map<Token, prefix_parse_fn> _prefix_fns;
  std::unordered_map<Token, infix_parse_fn> _infix_fns;
  std::string _filename;
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
  parse_tree::function_ptr function();
  parse_tree::import_ptr import();
  std::vector<parse_tree::variable> function_params();
  std::vector<parse_tree::element_ptr> statements();
  std::tuple<uint64_t, std::vector<uint64_t>> accessor_lit();
  parse_tree::element_ptr statement();
  parse_tree::element_ptr assignment();
  parse_tree::element_ptr if_statement();
  parse_tree::element_ptr while_statement();
  parse_tree::element_ptr for_statement();
  parse_tree::element_ptr expression_statement();
  parse_tree::element_ptr return_statement();
  parse_tree::expr_ptr conditional();
  parse_tree::expr_ptr expression(precedence precedence);
  parse_tree::expr_ptr identifier();
  parse_tree::expr_ptr number();
  parse_tree::expr_ptr str();
  parse_tree::expr_ptr prefix_expr();
  parse_tree::expr_ptr grouped_expr();
  parse_tree::expr_ptr array();
  parse_tree::expr_ptr infix_expr(parse_tree::expr_ptr left);
  parse_tree::expr_ptr index_expr(parse_tree::expr_ptr array);
  parse_tree::expr_ptr call_expr(parse_tree::expr_ptr function);
  std::vector<parse_tree::expr_ptr> expression_list();
  std::tuple<bool, std::string> locate_import(std::vector<std::string> &paths,
                                              std::string &target);
};
} // namespace compiler

#endif