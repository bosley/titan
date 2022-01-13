#include "titan.hpp"
#include "instructions.hpp"
#include "lexer.hpp"
#include "tokens.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

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
} // namespace

titan::titan() : _run(true), _analyze(false), _execute(true), _is_repl(true) {}

int titan::do_repl()
{
  std::cout << "Repl : a = " << _analyze << ", e = " << _execute << std::endl;

  /*
    Needs to be updated to be more aware of syntax so we can have multilined
    input
  */

  std::string line;
  _current_file.line = 1;

  while (_run) {

    std::cout << "> ";
    std::getline(std::cin, line);

    if (!is_processable(line)) {
      continue;
    }

    if (!run_line(line)) {
      // Report failure
      //
      //  _current_file.col will contain the col position of failure
      //  in the future when this thing is more aware we can split it by
      //  \n and show the exact "line" of failure too
      return 1;
    }
    _current_file.line++;
  }
  return 0;
}

int titan::do_run(std::vector<std::string> files)
{
  _is_repl = false;
  std::cout << "Run : a = " << _analyze << ", e = " << _execute << ", "
            << files.size() << " files given" << std::endl;

  for (auto &f : files) {
    std::cout << "\t" << f << std::endl;
    if (!run_file(f)) {
      return 1;
    }
  }
  return 0;
}

bool titan::run_file(std::string_view file)
{
  if (!std::filesystem::is_regular_file(file)) {
    std::cout << "Given item : " << file << " is not a file" << std::endl;
    return false;
  }

  std::ifstream ifs;
  ifs.open(file);

  if (!ifs.is_open()) {
    std::cout << "Unable to open item : " << file << std::endl;
    return false;
  }

  _current_file.name = file;
  _current_file.line = 0;
  _current_file.col = 0;

  std::string line;
  while (std::getline(ifs, line)) {
    _current_file.line++;

    trim_line(line);

    if (!is_processable(line)) {
      continue;
    }

    if (!run_line(line)) {
      //  Report the error
      //  _current_file.col will be set to error location
      //  _current_file.line will be the line number
      return false;
    }
  }

  ifs.close();
  return true;
}

bool titan::run_line(std::string_view line)
{
  std::cout << "Run line : " << line << std::endl;

  // Lex the line into tokens

  lexer l;
  auto tokens = l.lex(_current_file.line, std::string(line));

  if (tokens.empty()) {
    return true;
  }

  for (auto &t : tokens) {
    std::cout << token_to_str(t) << " ";
  }
  std::cout << std::endl;

  // Parse the line into an instruction

  // If analyze - Analyze the instruction for semantics

  // If execute - Execute the instruction

  // Check to see if instruction terminates _run

  return true;
}

} // namespace titan
