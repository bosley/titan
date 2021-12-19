#include "compiler/lexer.hpp"
#include "compiler/parser.hpp"

#include <filesystem>
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
  std::cout << program_name << std::endl;
  std::cout << "\nUsage:\n";
  std::cout << "  " << program_name
            << " [<include-directories>]  <source-file> <source-file>..."
            << std::endl;
  std::cout << "\nOptions:\n";
  std::cout << "  -h --help      Show this help screen\n";
  std::cout << "  -i --include   Include a ':' delimited directory list\n";
  std::cout << "\nExample:\n  " << program_name
            << " -i /path/to/directory:/path/to/another/directory main.tl"
            << std::endl;
  return;
}

/*
  Builds the list of include directories from -i or --include and ensures
  that each item given is a directory
*/
void parse_includes(std::string includes) {
  size_t pos = 0;
  std::string directory;
  while ((pos = includes.find(":")) != std::string::npos) {
    include_directories.emplace_back(includes.substr(0, pos));
    includes.erase(0, pos + 1);
  }
  include_directories.emplace_back(includes);
  for (auto &expected_directory : include_directories) {
    if (!std::filesystem::is_directory(expected_directory)) {
      std::cout << "Given include item \"" << expected_directory
                << "\" is not a directory" << std::endl;
      std::exit(0);
    }
  }
}

/*
  Parses input arguments
*/
void parse_args(std::vector<std::string> args) {
  program_name = args[0];
  for (size_t idx = 1; idx < args.size(); idx++) {
    auto &arg = args[idx];
    if (arg == "-h" || arg == "--help") {
      show_usage();
      std::exit(0);
    }
    if (arg == "-i" || arg == "--include") {
      if (args.size() <= idx + 1) {
        std::cout << "No value given to \"" << arg << "\"" << std::endl;
        std::exit(0);
      }
      parse_includes(args[idx + 1]);
      idx += 1;
      continue;
    }
    // After arguments, everything will be file names
    filenames.push_back(arg);
  }
}

int main(int argc, char **argv) {

  parse_args(std::vector<std::string>(argv, argv + argc));

  constexpr auto import_file =
      [](std::string file) -> std::vector<compiler::TD_Pair> {
    compiler::lexer lexer;
    if (!lexer.load_file(file)) {
      std::exit(-1);
    }
    std::vector<compiler::TD_Pair> td;
    if (!lexer.lex(td)) {
      std::exit(-1);
    }
    return td;
  };

  for (auto &file : filenames) {

    auto files_tokens = import_file(file);

    compiler::parser parser;

    std::vector<compiler::parse_tree::toplevel *> p_tree =
        parser.parse(include_directories, import_file, files_tokens);

    std::cout << "Top level items : " << p_tree.size() << std::endl;
  }

  return 0;
}