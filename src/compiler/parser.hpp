#ifndef COMPILER_PARSER_HPP
#define COMPILER_PARSER_HPP

#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "common.hpp"
#include "parsetree.hpp"
#include "tokens.hpp"

namespace compiler {

class parser {
public:
  enum class precedence {
    LOWEST,
    EQUALS,       // ==
    LESS_GREATER, // > <
    SUM,          // +
    PROD,         // *
    PREFIX,       // -a or !a
    CALL,         // call()
    INDEX         // []
  };
  parser();

  std::vector<parse_tree::toplevel *>
  parse(std::string filename, std::vector<std::string> &include_directories,
        std::function<std::vector<TD_Pair>(std::string)> import_function,
        std::vector<TD_Pair> &tokens);

  bool is_okay() const { return _parser_okay; }

private:
  typedef parse_tree::expression *(parser::*prefix_parse_fn)();
  typedef parse_tree::expression *(parser::*infix_parse_fn)(
      parse_tree::expression *);

  bool _parser_okay;
  size_t _idx;
  size_t _mark;
  std::vector<TD_Pair> *_tokens;
  std::unordered_map<Token, prefix_parse_fn> _prefix_fns;
  std::unordered_map<Token, infix_parse_fn> _infix_fns;
  std::set<std::string> _imported_objects;
  std::string _filename;
  void prev();
  void advance();
  void mark();
  void reset();
  void unset();
  void die(std::string error);
  void expect(Token token, std::string error, size_t ahead = 0);
  TD_Pair peek(size_t ahead = 1);
  precedence peek_precedence();
  parse_tree::toplevel *function();
  parse_tree::toplevel *import_stmt();
  std::vector<parse_tree::variable> function_params();
  std::vector<parse_tree::element *> statements();
  uint64_t accessor_depth();
  parse_tree::element *statement();
  parse_tree::element *assignment();
  parse_tree::element *reassignment_statement();
  parse_tree::element *if_statement();
  parse_tree::element *while_statement();
  parse_tree::element *expression_statement();
  parse_tree::element *return_statement();
  parse_tree::expression *conditional();
  parse_tree::expression *expression(precedence precedence);
  parse_tree::expression *identifier();
  parse_tree::expression *number();
  parse_tree::expression *str();
  parse_tree::expression *prefix_expr();
  parse_tree::expression *grouped_expr();
  parse_tree::expression *array();
  parse_tree::expression *infix_expr(parse_tree::expression *left);
  parse_tree::expression *index_expr(parse_tree::expression *array);
  parse_tree::expression *call_expr(parse_tree::expression *function);
  std::vector<parse_tree::expression *> expression_list();
};
} // namespace compiler

#endif
