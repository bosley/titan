#include "compiler/lexer.hpp"
#include "compiler/parser.hpp"

#include "log/log.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

std::string program_name;
std::string working_directory;
std::vector<std::string> filenames;
std::vector<std::string> include_directories;

enum class LogLevel { TRACE, DEBUG, INFO, WARNING, ERROR, FATAL };

LogLevel logger_level;

std::unordered_map<std::string, LogLevel> logger_args;

} // namespace

void setup_logger()
{
  switch (logger_level) {
  case LogLevel::TRACE:
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);
    break;
  case LogLevel::DEBUG:
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::debug);
    break;
  case LogLevel::INFO:
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::info);
    break;
  case LogLevel::WARNING:
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::warning);
    break;
  case LogLevel::ERROR:
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::error);
    break;
  case LogLevel::FATAL:
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::fatal);
    break;
  default:
    std::cerr << "Internal error : Unable to map log level to logger"
              << std::endl;
    std::exit(1);
    break;
  };
}

void show_usage()
{
  std::cout << program_name << std::endl;
  std::cout << "\nUsage:\n";
  std::cout << "  " << program_name
            << " [<include-directories>]  <source-file> <source-file>..."
            << std::endl;
  std::cout << "\nOptions:\n";
  std::cout << "  -h --help             Show this help screen\n";
  std::cout << "  -l --log <level>      Set logging level\n";
  std::cout << "\n     Levels:\n";

  for (auto &i : logger_args) {
    std::cout << "            " << i.first << std::endl;
  }

  std::cout
      << "\n  -i --include          Include a ':' delimited directory list\n";
  std::cout << "\nExample:\n  " << program_name
            << " -i /path/to/directory:/path/to/another/directory main.tl"
            << std::endl;
  return;
}

/*
  Builds the list of include directories from -i or --include and ensures
  that each item given is a directory
*/
void parse_includes(std::string includes)
{
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

void set_logger_level(std::string level)
{
  if (logger_args.find(level) == logger_args.end()) {
    std::cout << "Invalid argument \"" << level
              << "\" for log level. Use -h for help" << std::endl;
    std::exit(1);
  }

  logger_level = logger_args[level];
}

/*
  Parses input arguments
*/
void parse_args(std::vector<std::string> args)
{
  program_name = args[0];
  for (size_t idx = 1; idx < args.size(); idx++) {
    auto &arg = args[idx];
    if (arg == "-h" || arg == "--help") {
      show_usage();
      std::exit(0);
    }
    if (arg == "-l" || arg == "--log") {
      if (args.size() <= idx + 1) {
        std::cout << "No value given to \"" << arg << "\"" << std::endl;
        std::exit(0);
      }
      set_logger_level(args[idx + 1]);
      idx += 1;
      continue;
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

int main(int argc, char **argv)
{
  logger_args["trace"] = LogLevel::TRACE;
  logger_args["debug"] = LogLevel::DEBUG;
  logger_args["info"] = LogLevel::INFO;
  logger_args["warning"] = LogLevel::WARNING;
  logger_args["error"] = LogLevel::ERROR;
  logger_args["fatal"] = LogLevel::FATAL;

  logger_level = LogLevel::FATAL;

  parse_args(std::vector<std::string>(argv, argv + argc));

  setup_logger();

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

    auto p_tree =
        parser.parse(file, include_directories, import_file, files_tokens);

    LOG(INFO) << "Top level items : " << p_tree.size() << std::endl;
  }

  return 0;
}
