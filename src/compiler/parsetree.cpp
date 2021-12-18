#include "parsetree.hpp"

namespace compiler {
namespace parse_tree {

void assignment::visit(visitor &v) { v.accept(*this); }

} // namespace parse_tree
} // namespace compiler