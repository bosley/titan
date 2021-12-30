#include "analyzer.hpp"

namespace compiler {


analyzer::analyzer(std::vector<parse_tree::toplevel_ptr> &trees) : _trees(trees) {

}

bool analyzer::analyze() {

  return false;
}

void analyzer::accept(parse_tree::assignment_statement &stmt) {}
void analyzer::accept(parse_tree::expression_statement &stmt) {}
void analyzer::accept(parse_tree::if_statement &stmt) {}
void analyzer::accept(parse_tree::while_statement &stmt) {}
void analyzer::accept(parse_tree::for_statement &stmt) {}
void analyzer::accept(parse_tree::return_statement &stmt) {}

}
