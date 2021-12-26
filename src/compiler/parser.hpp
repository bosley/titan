#ifndef COMPILER_PARSER_HPP
#define COMPILER_PARSER_HPP

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
  typedef parse_tree::expr* (parser::*prefix_parse_fn)();
  typedef parse_tree::expr* (parser::*infix_parse_fn)(parse_tree::expr* );


  bool _parser_okay;
  size_t _idx;
  std::vector<TD_Pair> *_tokens;
  std::unordered_map<Token, prefix_parse_fn> _prefix_fns; 
  std::unordered_map<Token, infix_parse_fn>  _infix_fns; 
  std::set<std::string> _imported_objects;
  std::string _filename;
  void prev();
  void advance();
  void die(std::string error);
  void expect(Token token, std::string error, size_t ahead = 0);
  TD_Pair peek(size_t ahead = 1);
  precedence peek_precedence();
  parse_tree::toplevel *function();
  parse_tree::toplevel *import_stmt();
  std::vector<parse_tree::variable> function_params();
  std::vector<parse_tree::element *> statements();
  parse_tree::element *statement();
  parse_tree::element *assignment();
  parse_tree::element *if_statement();
  parse_tree::element *else_if_statement();
  parse_tree::element *else_statement();
  parse_tree::element *loop();
  parse_tree::element *expression_statement();
  parse_tree::expr *expression(precedence precedence);
  parse_tree::expr *identifier();
  parse_tree::expr *number(); 
  parse_tree::expr *str();
  parse_tree::expr *prefix_expr();
  parse_tree::expr *grouped_expr();
  parse_tree::expr *array();
  parse_tree::expr *infix_expr(parse_tree::expr* left);
  parse_tree::expr *index_expr(parse_tree::expr* array);
  parse_tree::expr *call_expr(parse_tree::expr* function);
  std::vector<parse_tree::expr *> expression_list();
};
} // namespace compiler

#endif


