#include "compiler/lexer.hpp"

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

  if (!lexer.load_files(filenames)) {
    return -1;
  }

  std::vector<compiler::TD_Pair> token_data_pairs;
  switch (lexer.lex(token_data_pairs)) {
  case compiler::lexer::LexerResult::WARNINGS:
    return -1;
  case compiler::lexer::LexerResult::ERRORS:
    return -1;
  case compiler::lexer::LexerResult::OKAY:
    break;
  default:
    std::cerr << "Default reached main : lexer.lex(td_pair)" << std::endl;
    return -1;
    break;
  }

  // Send the token_data_pairs to the parser

  std::cout << "Got : " << token_data_pairs.size() << " tokens\n";

  for (auto &tok : token_data_pairs) {
    std::cout << " " << token_to_str(tok);
  }
  std::cout << std::endl;

  return 0;
}