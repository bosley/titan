#ifndef EXEC_HPP
#define EXEC_HPP

#include "env.hpp"
#include "lang/instructions.hpp"
#include "lang/tokens.hpp"
#include "types/types.hpp"

#include <stack>
#include <string>
#include <vector>

namespace titan
{

//  Signals that can be emitted by the exec object
enum class exec_sig {
  EXIT = 0
};

//  Callback interface that receives signals and messages
//  from the exec object
class exec_cb_if {
public:
  virtual void signal(exec_sig sig, const std::string& msg) = 0;
};

//  Execute instructions
//    calls back on exec_cb_f 
//    and works env
class exec : public instructions::ins_receiver {
public:
  exec(exec_cb_if &cb, env &env);

  void set_operating_space(const std::string& space);

  virtual void receive(instructions::define_user_struct &ins) override;
  virtual void receive(instructions::scope_change &ins) override;
  virtual void receive(instructions::assignment_instruction &ins) override;
  virtual void receive(instructions::expression_instruction &ins) override;
  virtual void receive(instructions::if_instruction &ins) override;
  virtual void receive(instructions::while_instruction &ins) override;
  virtual void receive(instructions::for_instruction &ins) override;
  virtual void receive(instructions::return_instruction &ins) override;
  virtual void receive(instructions::import &ins) override;
  virtual void receive(instructions::function &ins) override;

private:
  exec_cb_if *_cb;
  env *_env;
  std::string _space;
  object* execute_expression(instructions::expression* expr, bool clone_variables=false);
  object* handle_function_call(instructions::function_call_expr* call);

  object* perform_unary_op(object* rhs, Token op);
  object* perform_op(object* lhs, object* rhs, Token op);

  object* assign(object* lhs, object* rhs);

  object* equality_check(object* lhs, object* rhs);
  object* add_eq(object* lhs, object* rhs);
  object* sub_eq(object* lhs, object* rhs);
  object* div_eq(object* lhs, object* rhs);
  object* mul_eq(object* lhs, object* rhs);
  object* mod_eq(object* lhs, object* rhs);
  object* pow_eq(object* lhs, object* rhs);
  object* lsh_eq(object* lhs, object* rhs);
  object* rsh_eq(object* lhs, object* rhs);
  object* hat_eq(object* lhs, object* rhs);
  object* pipe_eq(object* lhs, object* rhs);
  object* tilde_eq(object* lhs, object* rhs);
  object* ampersand_eq(object* lhs, object* rhs);
  object* not_equal(object* lhs, object* rhs);
  object* lt(object* lhs, object* rhs);
  object* gt(object* lhs, object* rhs);
  object* lte(object* lhs, object* rhs);
  object* gte(object* lhs, object* rhs);
  object* rsh(object* lhs, object* rhs);
  object* lsh(object* lhs, object* rhs);
  object* add(object* lhs, object* rhs);
  object* sub(object* lhs, object* rhs);
  object* div(object* lhs, object* rhs);
  object* mul(object* lhs, object* rhs);
  object* mod(object* lhs, object* rhs);
  object* pow(object* lhs, object* rhs);
  object* ampersand(object* lhs, object* rhs);
  object* logical_or(object* lhs, object* rhs);
  object* logical_and(object* lhs, object* rhs);
  object* pipe(object* lhs, object* rhs);

  object* hat(object* rhs);
  object* tilde(object* rhs);
  object* sub(object* rhs);
  object* negate(object* rhs);
};

}

#endif
