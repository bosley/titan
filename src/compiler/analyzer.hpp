#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "parsetree.hpp"

#include <vector>

namespace compiler {

class analyzer : private parse_tree::visitor {
public:
  analyzer(std::vector<parse_tree::toplevel_ptr> &parse_tree);

  bool analyze();

private:

  // Flags that need to be true at the end of analyzing 
  // the parse trees
  //
  struct check_flags {

    bool entry_method_exists;
    
  };


  std::vector<parse_tree::toplevel_ptr> &_tree;

  parse_tree::function* _current_function;


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
