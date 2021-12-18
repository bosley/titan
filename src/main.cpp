#include "compiler/lexer.hpp"
#include "compiler/parser.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

std::string program_name;
std::string working_directory;
std::vector<std::string> filenames;
std::vector<std::string> include_directories;

} // namespace

void show_usage() {

  std::cout << "usage: " << program_name << "<file_names...>" << std::endl;
  return;
}

void parse_args(std::vector<std::string> args) {

  program_name = args[0];

  for (size_t idx = 1; idx < args.size(); idx++) {

    auto &arg = args[idx];

    if (arg == "-h" || arg == "--help") {
      show_usage();
      std::exit(0);
    }

    // After arguments, everything will be file names
    filenames.push_back(arg);
  }
}

int main(int argc, char **argv) {

  parse_args(std::vector<std::string>(argv, argv + argc));

  compiler::lexer lexer;

  for (auto &file : filenames) {
    if (!lexer.load_file(file)) {
      return -1;
    }
    std::vector<compiler::TD_Pair> token_data_pairs;
    if (!lexer.lex(token_data_pairs)) {
      std::cout << "Failed to lex : " << file << std::endl;
    }

    for (auto &td : token_data_pairs) {
      std::cout << " " << compiler::token_to_str(td);
    }
    std::cout << std::endl;

    // Parser will eventually emit a tree
    compiler::parser().parse(token_data_pairs);
  }

  return 0;
}