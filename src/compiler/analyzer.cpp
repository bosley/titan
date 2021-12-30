#include "analyzer.hpp"

namespace compiler {


analyzer::analyzer(std::vector<parse_tree::toplevel_ptr> &trees) : _trees(trees) {

}

bool analyzer::analyze() {

  return false;
}


}
