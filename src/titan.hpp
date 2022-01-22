#ifndef TITAN_HPP
#define TITAN_HPP

#include "exec/env.hpp"
#include "exec/exec.hpp"
#include "lang/tokens.hpp"
#include "lang/parser.hpp"
#include "types/types.hpp"

#include <string>
#include <vector>

namespace titan {

class titan : public exec_cb_if {
public:
  titan();
  ~titan();
  void set_analyze(bool analyze) { _analyze = analyze; }
  void set_execute(bool execute) { _execute = execute; }

  int do_repl();
  int do_run(std::string file);
  void set_include_dirs(std::vector<std::string> dir_list);

  // Install an external function to the environment
  bool install_xfunc(const std::string& name, env::xfunc *tei)
  {
    if(!tei){ return false; }
    return _environment.add_xfunc(name, tei);
  }

  object* get_env_var(const std::string& name)
  {
    return _environment.get_variable("main", name);
  }

  bool new_env_var(const std::string& name, object* var) 
  {
    return _environment.new_variable("main", name, var);
  }

  virtual void signal(exec_sig sig, const std::string& msg) override;

private:
  bool _run;
  bool _analyze;
  bool _execute;
  bool _is_repl;

  struct fp_info {
    std::string_view name;
    size_t line;
    size_t col;
  };

  fp_info _current_file;

  env _environment;
  parser _parser;
  exec * _executor;

  bool run_tokens(std::vector<TD_Pair> tokens);
};

} // namespace titan

#endif
