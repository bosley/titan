#include "titan.hpp"
#include "lang/instructions.hpp"
#include "lang/lexer.hpp"
#include "lang/tokens.hpp"
#include "analyze/analyzer.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

// Built in methods that need to be added to the env
//
#include "built_in/puts.hpp"


namespace titan {

namespace {

void trim_line(std::string &line)
{
  line.erase(std::find_if(line.rbegin(), line.rend(),
                          [](int ch) { return !std::isspace(ch); })
                 .base(),
             line.end());
  line.erase(line.begin(), find_if_not(line.begin(), line.end(),
                                       [](int c) { return isspace(c); }));
}

bool is_processable(std::string &line)
{
  if (line.empty()) {
    return false;
  }
  if (line.size() >= 2 && line[0] == '/' && line[1] == '/') {
    return false;
  }
  return true;
}

std::vector<TD_Pair> lex_file(std::string file) 
{
  if (!std::filesystem::is_regular_file(file)) {
    std::cout << "Importer : Given item : " << file << " is not a file" << std::endl;
    return {};
  }

  std::ifstream ifs;
  ifs.open(file);

  if (!ifs.is_open()) {
    std::cout << "Importer : Unable to open item : " << file << std::endl;
    return {};
  }

  std::string line;
  size_t line_no = 0;
  std::vector<TD_Pair> result;
  while (std::getline(ifs, line)) {
    line_no++;
    trim_line(line);
    if (!is_processable(line)) {
      continue;
    }

    lexer l;
    auto tokens = l.lex(line_no, line);
    if(!tokens.empty()) {
      result.insert(result.end(), tokens.begin(), tokens.end());
    }
  }

  ifs.close();
  return result;
}

imports g_importer(lex_file, {});

} // namespace

titan::titan()
    : _run(true), _analyze(false), _execute(true), _is_repl(true),
      _parser(g_importer), _executor(nullptr)
{
  _executor = new exec(*this, _environment);

  // Puts
  _built_ins.emplace_back(new built_in::puts());
  _environment.add_xfunc("__puts", _built_ins.back());
}

titan::~titan()
{
  delete _executor;
  for(auto& f : _built_ins) {
    delete f;
  }
}

int titan::do_repl()
{
  std::cout << "Repl : a = " << _analyze << ", e = " << _execute << std::endl;

  /*
    Needs to be updated to be more aware of syntax so we can have multilined
    input
  */

  std::string line;
  _current_file.line = 1;
  _current_file.name = "repl";

  while (_run) {

    std::cout << "> ";
    std::getline(std::cin, line);

    if (!is_processable(line)) {
      continue;
    }

    lexer l;
    auto tokens = l.lex(_current_file.line, std::string(line));

    if (!run_tokens(tokens)) {
      // Report failure
      //  _current_file.col will contain the col position of failure
      //  in the future when this thing is more aware we can split it by
      //  \n and show the exact "line" of failure too
      return 1;
    }
    _current_file.line++;
    std::cout << std::endl;
  }

  // TODO: Return the environment's return code
  return 0;
}

int titan::do_run(std::string file)
{
  _is_repl = false;

  if (!std::filesystem::is_regular_file(file)) {
    std::cout << "Given item : " << file << " is not a file" << std::endl;
    return 1;
  }
  
  if(!run_tokens(lex_file(file))) {
    // Report failure
    return 1;
  }

  // TODO: Return the environment's return code
  return 0;
}

void titan::set_include_dirs(std::vector<std::string> dir_list)
{
  g_importer.include_directories = dir_list;
}

bool titan::run_tokens(std::vector<TD_Pair> tokens) 
{
  if (tokens.empty()) {
    return true;
  }

  for (auto &t : tokens) {
    std::cout << token_to_str(t) << " ";
  }
  std::cout << std::endl;

  // Generate instruction(s) from token stream
  auto instructions = _parser.parse(std::string(_current_file.name), tokens);

  std::cout << "Got : " << instructions.size() << " instructions" << std::endl;

  // If analyze - Analyze the instruction for semantics

  // If execute - Execute the instruction
  if (_analyze) {
    analyzer a(instructions);
    if(!a.analyze()) {
      std::cout << "Analyzer has detected a problem" << std::endl;
      return false;
    }
  }

  // Run instruction(s)
  if(_execute) {
    for(auto& ins : instructions) {
      ins->visit(*_executor);
    }
  }

  return true;
}

void titan::signal(exec_sig sig, const std::string& msg)
{
  switch(sig)
  {
  case exec_sig::EXIT:
    std::cout << "Received EXIT signal >> " << msg << std::endl;
    break;
  }
}

} // namespace titan
