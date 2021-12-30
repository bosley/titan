#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "parsetree.hpp"

#include <vector>

namespace compiler {

class analyzer : private parse_tree::visitor {
public:
  analyzer(std::vector<parse_tree::toplevel_ptr> &parse_trees);

  bool analyze();

private:

  // Flags that need to be true at the end of analyzing 
  // the parse trees
  //
  struct check_flags {

    bool entry_method_exists;
    
  };

  //  Information regarding the last function analyzed
  //
  struct analyzed_function {
    std::string file;
    std::string name;
    size_t line;
    size_t col;
    parse_tree::variable_types indicated_return_type;
    bool all_exits_match_return_type;
  };

  std::vector<parse_tree::toplevel_ptr> &_trees; // trees to analyze

  analyzed_function current_function;


  virtual void accept(parse_tree::assignment_statement &stmt) override;
  virtual void accept(parse_tree::expression_statement &stmt) override;
  virtual void accept(parse_tree::if_statement &stmt) override;
  virtual void accept(parse_tree::while_statement &stmt) override;
  virtual void accept(parse_tree::for_statement &stmt) override;
  virtual void accept(parse_tree::return_statement &stmt) override;
};

}

#endif
