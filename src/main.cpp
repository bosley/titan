#include <iostream>
#include <string>
#include <vector>

#include "titan.hpp"

/*
    - repl
      If no files are given, REPL begins

    - check
      If check is given, parse all source files given and perform semantic anlysis

    - execute
      Execute given source file(s)
 */

int show_usage(std::string_view program_name)
{
  std::cout << "Usage: " << program_name << " [args] [source file]" << std::endl; 
  return 0;
}

int main(int argc, char**argv) 
{
  std::vector<std::string> arguments(argv, argv + argc);  

  bool analyze = false;
  bool execute = true;
  std::vector<std::string> sources;
  std::string_view program_name = arguments[0];

  for(size_t idx = 1; idx < arguments.size(); ++idx) {

    auto &arg = arguments[idx];

    if(arg == "-h" || arg == "--help") {
      return show_usage(program_name);
    }

    if(arg == "-a" || arg == "--analyze") {
      analyze = true;
      continue;
    }

    if(arg == "-n" || arg == "--norun") {
      execute = false;
      continue;
    }

    sources.push_back(arg);
  }

  if(!analyze && !execute) {
    std::cout << "Nothing to do" << std::endl;
    return 0;
  }

  titan::titan t;
  t.set_analyze(analyze);
  t.set_execute(execute);

  if(sources.empty()) {
    return t.do_repl();
  }
  
  return t.do_run(sources);
}
