#include "exec.hpp"

#include <iostream>

namespace titan
{

exec::exec(exec_cb_if &cb, env &env) : _cb(&cb), _env(&env)
{

}

void exec::receive(instructions::define_user_struct &ins) 
{
  std::cout << "EXEC : define user struct" << std::endl;
}

void exec::receive(instructions::assignment_instruction &ins) 
{
  std::cout << "EXEC : assignment" << std::endl;
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

  // New top level scope

  for(auto& var : ins.parameters) {
    // Load parameters by the name given in var
  }

  for(auto& ins : ins.instruction_list) {
    ins->visit(*this);
  }

  // Pop top level scope
}

}
