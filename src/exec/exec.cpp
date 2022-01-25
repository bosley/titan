#include "exec.hpp"

#include <iostream>
#include <optional>
#include <tuple>
#include <sstream>

namespace titan
{

namespace 
{
inline static object* convert_raw_int_to_obj(const std::string &data)
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

inline static object* convert_str_to_float(const std::string &data)
{
  double value = 0.00;
  std::istringstream iss(data);
  iss >> value;
  return new object_float(value);
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
  if (!_env->new_variable(_space, ins.var->name,
                          new object_var(execute_expression(ins.expr.get())))) {
    std::cout << "Unable to create variable - raise an error" << std::endl;
  }
}

void exec::receive(instructions::expression_instruction &ins)
{
  std::cout << "EXEC : expression" << std::endl;

  object * result = execute_expression(ins.expr.get());
  if(!result) {
    std::cout << "Exec expression no result" << std::endl;
    _object_stack.push(new object_nil());
    return;
  }
  _object_stack.push(result);
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
object *exec::execute_expression(instructions::expression *expr)
{
  switch (expr->type) {
  case instructions::node_type::ROOT:
    break;
  case instructions::node_type::CALL:
  {
    object * result = handle_function_call(
        reinterpret_cast<instructions::function_call_expr*>(expr));
    if(result) {
      return result;
    }
    return new object_nil();
  }
  case instructions::node_type::ARRAY_IDX:
    break;
  case instructions::node_type::INFIX:
  {
    auto infix_expr = reinterpret_cast<instructions::infix_expr*>(expr);
    auto lhs = execute_expression(infix_expr->left.get());
    auto rhs = execute_expression(infix_expr->right.get());
    return perform_op(lhs, rhs, infix_expr->tok_op);
  }
  case instructions::node_type::PREFIX:
  {
    auto prefix_expr = reinterpret_cast<instructions::prefix_expr*>(expr);
    auto rhs = execute_expression(prefix_expr->right.get());
    return perform_unary_op(rhs, prefix_expr->tok_op);
  }
  case instructions::node_type::ID:
  {
    // TODO : 
    //
    // Split space of access from ID ( some::path::id ) 
    // so we can access the specific space. 
    //  > Currently use the active scope
    //
    // The ID and space could be built in the parse stage to save us
    // doing this every time. (Significant speedup)
    //
    // This will mean we modify instructions::expression to hold
    // a 'path' 
    object * var = _env->get_variable(_space, expr->value);

    if(!var) {
      return new object_nil();
    }
    return var;
  }
  case instructions::node_type::RAW_STRING:
    return new object_str(expr->value);
  case instructions::node_type::RAW_FLOAT:
    return convert_str_to_float(expr->value);
  case instructions::node_type::RAW_NUMBER:
    return convert_raw_int_to_obj(expr->value);
  case instructions::node_type::ARRAY:
    break;
  } // Switch
  std::cout << "Not handled in execute_expression" << std::endl;
  return nullptr;
}

object *exec::handle_function_call(instructions::function_call_expr* call)
{
  const std::string& name = call->fn->value;

  //  Build parameters
  //
  std::vector<object*> params;
  for(auto& param : call->params) {
    params.push_back(execute_expression(param.get()));
  }

  //  Check for built in methods
  //
  {
    auto func = _env->get_external_function(name);
    if(func) {
      func->parameters = params;
      func->execute();
    
      auto result = func->result;
      func->result = nullptr;
      func->parameters.clear();
      return result;
    }
  }


  //  Check built in methods
  //
  
  // TODO : Search user made functions and execute it

  return new object_nil();
}

object *exec::perform_unary_op(object *rhs, Token op)
{
  switch (op) {
  case Token::TILDE:
    return tilde(rhs);
  case Token::EXCLAMATION:
    return negate(rhs);
  case Token::SUB:
    return sub(rhs);
  default:
    return nullptr;
  };
}

object *exec::perform_op(object *lhs, object *rhs, Token op)
{
  switch(op) {
  case Token::EQ: return assign(lhs, rhs);
  default:
    return nullptr;
  };
}

object* exec::assign(object *lhs, object* rhs)
{
  // Lhs should be a var

  if(lhs->type != obj_type::VAR) { 

    // TODO: Raise an error here
    return new object_i8(-1);
  }

  auto var = reinterpret_cast<object_var*>(lhs);
  var->value = object_ptr(rhs->clone());

  return new object_i8(1);
}

object* exec::equality_check(object* lhs, object* rhs){ return new object_nil(); }
object* exec::add_eq(object* lhs, object* rhs){ return new object_nil(); }
object* exec::sub_eq(object* lhs, object* rhs){ return new object_nil(); }
object* exec::div_eq(object* lhs, object* rhs){ return new object_nil(); }
object* exec::mul_eq(object* lhs, object* rhs){ return new object_nil(); }
object* exec::mod_eq(object* lhs, object* rhs){ return new object_nil(); }
object* exec::pow_eq(object* lhs, object* rhs){ return new object_nil(); }
object* exec::lsh_eq(object* lhs, object* rhs){ return new object_nil(); }
object* exec::rsh_eq(object* lhs, object* rhs){ return new object_nil(); }
object* exec::hat_eq(object* lhs, object* rhs){ return new object_nil(); }
object* exec::pipe_eq(object* lhs, object* rhs){ return new object_nil(); }
object* exec::tilde_eq(object* lhs, object* rhs){ return new object_nil(); }
object* exec::ampersand_eq(object* lhs, object* rhs){ return new object_nil(); }
object* exec::not_equal(object* lhs, object* rhs){ return new object_nil(); }
object* exec::lt(object* lhs, object* rhs){ return new object_nil(); }
object* exec::gt(object* lhs, object* rhs){ return new object_nil(); }
object* exec::lte(object* lhs, object* rhs){ return new object_nil(); }
object* exec::gte(object* lhs, object* rhs){ return new object_nil(); }
object* exec::rsh(object* lhs, object* rhs){ return new object_nil(); }
object* exec::lsh(object* lhs, object* rhs){ return new object_nil(); }
object* exec::add(object* lhs, object* rhs){ return new object_nil(); }
object* exec::sub(object* lhs, object* rhs){ return new object_nil(); }
object* exec::div(object* lhs, object* rhs){ return new object_nil(); }
object* exec::mul(object* lhs, object* rhs){ return new object_nil(); }
object* exec::mod(object* lhs, object* rhs){ return new object_nil(); }
object* exec::pow(object* lhs, object* rhs){ return new object_nil(); }
object* exec::ampersand(object* lhs, object* rhs){ return new object_nil(); }
object* exec::logical_or(object* lhs, object* rhs){ return new object_nil(); }
object* exec::logical_and(object* lhs, object* rhs){ return new object_nil(); }
object* exec::pipe(object* lhs, object* rhs){ return new object_nil(); }
object* exec::hat(object* rhs){ return new object_nil(); }
object* exec::tilde(object* rhs){ return new object_nil(); }
object* exec::sub(object* rhs){ return new object_nil(); }
object* exec::negate(object* rhs){ return new object_nil(); }

}
