#include "instructions.hpp"

namespace titan {
namespace instructions {

void define_user_struct::visit(ins_receiver &ir) { ir.receive(*this); }
void assignment_instruction::visit(ins_receiver &ir) { ir.receive(*this); }
void expression_instruction::visit(ins_receiver &ir) { ir.receive(*this); }
void if_instruction::visit(ins_receiver &ir) { ir.receive(*this); }
void while_instruction::visit(ins_receiver &ir) { ir.receive(*this); }
void for_instruction::visit(ins_receiver &ir) { ir.receive(*this); }
void return_instruction::visit(ins_receiver &ir) { ir.receive(*this); }
void import::visit(ins_receiver &ir) { ir.receive(*this); }
void function::visit(ins_receiver &ir) { ir.receive(*this); }
void scope_change::visit(ins_receiver &ir) { ir.receive(*this); }

variable_types string_to_variable_type(const std::string &s)
{
  if (s == "u8") {
    return variable_types::U8;
  }
  if (s == "u16") {
    return variable_types::U16;
  }
  if (s == "u32") {
    return variable_types::U32;
  }
  if (s == "u64") {
    return variable_types::U64;
  }
  if (s == "i8") {
    return variable_types::I8;
  }
  if (s == "i16") {
    return variable_types::I16;
  }
  if (s == "i32") {
    return variable_types::I32;
  }
  if (s == "i64") {
    return variable_types::I64;
  }
  if (s == "float") {
    return variable_types::FLOAT;
  }
  if (s == "string") {
    return variable_types::STRING;
  }
  /*
   *    NOTE : User defined times not handled here
   *    TODO : When user created constructs are created we will
   *           need to do something to handle it here
   */
  return variable_types::UNDEF;
}

void display_expr_tree(const std::string &prefix, expression *n, bool is_left)
{
  if (!n) {
    return;
  }
  std::cout << prefix;
  std::cout << (is_left ? "├──" : "└──");

  if (n->type == node_type::CALL) {
    auto i = reinterpret_cast<function_call_expr *>(n);
    if (i->fn) {
      std::cout << " call<" << i->fn->value << ">" << std::endl;
    }
    else {
      std::cout << " call " << std::endl;
    }
  }
  else if (n->type == node_type::ARRAY_IDX) {
    auto i = reinterpret_cast<array_index_expr *>(n);
    if (i->arr && i->index) {
      std::cout << " " << i->arr->value << "[" << i->index->value << "]"
                << std::endl;
    }
    else {
      std::cout << " array[] " << std::endl;
    }
  }
  else if (n->type == node_type::INFIX) {
    auto i = reinterpret_cast<infix_expr *>(n);
    std::cout << " " << i->op << std::endl;
    display_expr_tree(prefix + (is_left ? "│   " : "    "), i->left.get(),
                      true);
    display_expr_tree(prefix + (is_left ? "│   " : "    "), i->right.get(),
                      false);
  }
  else if (n->type == node_type::PREFIX) {
    auto i = reinterpret_cast<prefix_expr *>(n);
    std::cout << " " << i->op << std::endl;
    display_expr_tree(prefix + (is_left ? "│   " : "    "), i->right.get(),
                      false);
  }
  else {
    std::cout << " " << n->value << std::endl;
  }
}

} // namespace instructions
} // namespace titan
