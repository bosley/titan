#ifndef EXEC_HPP
#define EXEC_HPP

#include "env.hpp"
#include "lang/instructions.hpp"

#include <string>

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

  virtual void receive(instructions::define_user_struct &ins) override;
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
  env _env;
};

}

#endif
