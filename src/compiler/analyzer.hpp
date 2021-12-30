#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "parsetree.hpp"

#include<vector>

namespace compiler {

class analyzer {
public:
  analyzer(std::vector<parse_tree::toplevel_ptr> &parse_trees);

  bool analyze();

private:
  std::vector<parse_tree::toplevel_ptr> &_trees;
};

}

#endif
