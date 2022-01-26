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
  int64_t value = 0;
  std::istringstream iss(data);
  iss >> value;
  return new object_int(value);
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
  auto result = execute_expression(ins.expr.get(), true);



  if (!_env->new_variable(_space, ins.var->name,
                          new object_var(execute_expression(ins.expr.get(), true)))) {
    std::cout << "Unable to create variable - raise an error" << std::endl;
  }
}

void exec::receive(instructions::expression_instruction &ins)
{
  std::cout << "EXEC : expression" << std::endl;

  object * result = execute_expression(ins.expr.get());
  if(!result) {
    std::cout << "Exec expression no result" << std::endl;

    // TODO : Throw some error 

    return;
  }
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
object *exec::execute_expression(instructions::expression *expr, bool clone_variables)
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
    auto rhs = execute_expression(infix_expr->right.get(), true);
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

    object *var = (clone_variables)
                      ? _env->get_variable(_space, expr->value)->clone()
                      : _env->get_variable(_space, expr->value);
    if(!var) {
      return new object_nil();
    }

    //  If we clone the variable we are attempting to get its value, so we must drill 
    //  into it ang ensure we are retrieving a raw value not a var
    //
    while(clone_variables && (var->type == obj_type::VAR)) {
      var = reinterpret_cast<object_var*>(var)->value.get();
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
  if(lhs->type != obj_type::VAR) { 
    std::cout << "LHS not a var????" << std::endl;
    return new object_int(0);
  }

  auto var = reinterpret_cast<object_var*>(lhs);

  if(var->value->type == rhs->type) {
    var->value = object_ptr(rhs->clone());
    return new object_int(1);
  }

  /*
        If the items aren't the same type, then we need to cast the items
        to whatever type is on the lhs if we can
  */
  switch(var->value->type) {

  //
  //  LHS is int
  //
  case obj_type::INT:
  {

    //
    //    Check rhs to see how to cast
    //
    switch(rhs->type) {
    case obj_type::INT:
    {
      auto r = reinterpret_cast<object_int*>(rhs);
      var->value = object_ptr(new object_int(r->value));
      return new object_int(1);
    }
    case obj_type::FLOAT:
    {
      auto r = reinterpret_cast<object_float*>(rhs);
      var->value = object_ptr(new object_int(r->value));
      return new object_int(1);
    }
    case obj_type::VAR:
    {
      auto r = reinterpret_cast<object_var*>(rhs);
      return assign(lhs, r->value.get()->clone());
    }
    default:
      //
      //  TODO: Throw an error here
      //
      std::cout << "Unhandled item in exec::assign - >>> A " << std::endl;
      return new object_int(0);
    };
  }

  //
  //  LHS is float
  //
  case obj_type::FLOAT:
  {
    switch(rhs->type) {
    case obj_type::INT:
    {
      auto r = reinterpret_cast<object_int*>(rhs);
      var->value = object_ptr(new object_float(r->value));
      return new object_int(1);
    }
    case obj_type::FLOAT:
    {
      auto r = reinterpret_cast<object_float*>(rhs);
      var->value = object_ptr(new object_float(r->value));
      return new object_int(1);
    }
    case obj_type::VAR:
    {
      auto r = reinterpret_cast<object_var*>(rhs);
      return assign(lhs, r->value.get()->clone());
    }
    default:
      //
      //  TODO: Throw an error here
      //
      std::cout << "Unhandled item in exec::assign - >>> A " << (int)rhs->type << std::endl;
      return new object_int(0);
    };
  }

  //
  //  LHS is string
  //
  case obj_type::STRING:
  {
    switch(rhs->type) {
    case obj_type::INT:
    {
      auto r = reinterpret_cast<object_int*>(rhs);
      var->value = object_ptr(new object_str(std::to_string(r->value)));
      return new object_int(1);
    }
    case obj_type::FLOAT:
    {
      auto r = reinterpret_cast<object_float*>(rhs);
      var->value = object_ptr(new object_str(std::to_string(r->value)));
      return new object_int(1);
    }
    case obj_type::VAR:
    {
      auto r = reinterpret_cast<object_var*>(rhs);
      return assign(lhs, r->value.get()->clone());
    }
    default:
      //
      //  TODO: Throw an error here
      //
      std::cout << "Unhandled item in exec::assign - >>> B " << (int)rhs->type << std::endl;
      return new object_int(0);
    };
  }
  case obj_type::VAR:
  {
    //auto l = reinterpret_cast<object_var*>(lhs);
    //return assign(l->value.get(), rhs);
  }
  default:
    //
    //  TODO: Throw an error here
    //
  std::cout << "LHS Item not handled in expr : " << (int)var->value->type << std::endl;
    return new object_int(0);
  };
}

object* exec::equality_check(object* lhs, object* rhs)
{
   
  return new object_nil();
}

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
