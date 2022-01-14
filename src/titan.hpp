#ifndef TITAN_HPP
#define TITAN_HPP

#include "env.hpp"
#include "tokens.hpp"
#include "parser.hpp"

#include <string>
#include <vector>

namespace titan {

class titan {
public:
  titan();
  void set_analyze(bool analyze) { _analyze = analyze; }
  void set_execute(bool execute) { _execute = execute; }

  int do_repl();
  int do_run(std::string file);
  void set_include_dirs(std::vector<std::string> dir_list);

  // Install an external function to the environment
  bool install_xfunc(env::xfunc *tei)
  {
    if(!tei){ return false; }
    return _environment.add_xfunc(tei);
  }

  std::optional<instructions::variable> get_env_var(std::string_view name)
  {
    return _environment.get_variable(name);
  }

  bool set_env_var(instructions::variable var, bool as_global) 
  {
    return _environment.set_variable(var, as_global);
  }

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

  bool run_tokens(std::vector<TD_Pair> tokens);
};

} // namespace titan

#endif
