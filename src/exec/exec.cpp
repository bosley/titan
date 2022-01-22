#include "exec.hpp"

#include <iostream>
#include <optional>
#include <tuple>
#include <sstream>

namespace titan
{

namespace 
{
object* convert_raw_int_to_obj(const std::string &data)
{
  long long value = 0;
  std::istringstream iss(data);
  iss >> value;

  if (value <= 0) {
    if (value <= std::numeric_limits<int8_t>::min()) {
      return new object_i8(static_cast<int8_t>(value));
    }
    else if (value <= std::numeric_limits<int16_t>::min()) {
      return new object_i16(static_cast<int16_t>(value));
    }
    else if (value <= std::numeric_limits<int32_t>::min()) {
      return new object_i32(static_cast<int32_t>(value));
    }
    else {
      return new object_i64(static_cast<int64_t>(value));
    }
  }
  else {
    if (value <= std::numeric_limits<uint8_t>::max()) {
      return new object_u8(static_cast<uint8_t>(value));
    }
    else if (value <= std::numeric_limits<uint16_t>::max()) {
      return new object_u16(static_cast<uint16_t>(value));
    }
    else if (value <= std::numeric_limits<uint32_t>::max()) {
      return new object_u32(static_cast<uint32_t>(value));
    }
    else {
      return new object_u64(static_cast<uint64_t>(value));
    }
  }
  return nullptr;
}
}

exec::exec(exec_cb_if &cb, env &env) : _cb(&cb), _env(&env), _space("GLOBAL")
{
  _env->new_space(_space);
}

void exec::set_operating_space(const std::string& op_space)
{
  _space = op_space;

  // If it fails, thats fine we just need to ensure that
  // it exists
  _env->new_space(_space);
}

void exec::receive(instructions::define_user_struct &ins) 
{
  std::cout << "EXEC : define user struct" << std::endl;
}

void exec::receive(instructions::scope_change &ins) 
{
  set_operating_space(ins.scope);
}

void exec::receive(instructions::assignment_instruction &ins)
{
  //std::cout << "EXEC : assignment" << std::endl;

  if (!_env->new_variable(_space, ins.var->name,
                          execute_expression(ins.expr.get()))) {

    std::cout << "Unable to create variable - raise an error" << std::endl;
  }
}

void exec::receive(instructions::expression_instruction &ins)
{
  std::cout << "EXEC : expression" << std::endl;
}

void exec::receive(instructions::if_instruction &ins)
{
  std::cout << "EXEC : if" << std::endl;
}

void exec::receive(instructions::while_instruction &ins)
{
  std::cout << "EXEC : while" << std::endl;
}

void exec::receive(instructions::for_instruction &ins)
{
  std::cout << "EXEC : for" << std::endl;
}

void exec::receive(instructions::return_instruction &ins)
{
  std::cout << "EXEC : return" << std::endl;
}

void exec::receive(instructions::import &ins)
{
  std::cout << "EXEC : import" << std::endl;
}

void exec::receive(instructions::function &ins)
{
  std::cout << "EXEC : fn" << std::endl;

  for(auto& var : ins.parameters) {
    // Load parameters by the name given in var
  }

  for(auto& ins : ins.instruction_list) {
    ins->visit(*this);
  }
}

/*
    Walk an expression and execute it.  

*/
object * exec::execute_expression(instructions::expression* expr)
{
  switch (expr->type) {
  case instructions::node_type::ROOT: break;
  case instructions::node_type::CALL: break;
  case instructions::node_type::ARRAY_IDX: break;
  case instructions::node_type::INFIX: break;
  case instructions::node_type::PREFIX: break;
  case instructions::node_type::ID: break;
  case instructions::node_type::RAW_FLOAT: break;
  case instructions::node_type::RAW_STRING: break;
  case instructions::node_type::RAW_NUMBER: return convert_raw_int_to_obj(expr->value);
  case instructions::node_type::ARRAY: break;
  } // Switch
  std::cout << "Not handled in execute_expression" << std::endl;
  return nullptr;
}

}
