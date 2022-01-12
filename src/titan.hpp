#ifndef TITAN_HPP
#define TITAN_HPP

#include <string>
#include <vector>

namespace titan
{

class titan
{
public:
  titan();
  void set_analyze(bool analyze){ _analyze = analyze; }
  void set_execute(bool execute){ _execute = execute; }

  int do_repl();
  int do_run(std::vector<std::string> files);

private:
  bool _run;
  bool _analyze;
  bool _execute;
  bool _is_repl;

  struct fp_info 
  {
    std::string_view name;
    size_t line;
    size_t col;
  };

  fp_info _current_file;

  bool run_file(std::string_view file_name);
  bool run_line(std::string_view line);
};

}

#endif
