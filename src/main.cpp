
#include "app.hpp"
#include "titan.hpp"
#include "log/log.hpp"

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

/*
    - repl
      If no files are given, REPL begins

    - check
      If check is given, parse all source files given and perform semantic
   anlysis

    - execute
      Execute given source file(s)
 */

namespace 
{
  enum class LogLevel { TRACE, DEBUG, INFO, WARNING, ERROR, FATAL };
  
  LogLevel logger_level;
  std::unordered_map<std::string, LogLevel> logger_args;
}

void setup_logger()
{
  switch (logger_level) {
  case LogLevel::TRACE:
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace,
                                        "[#severity] (#tag) #message");
    break;
  case LogLevel::DEBUG:
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::debug,
                                        "[#severity] (#tag) #message");
    break;
  case LogLevel::INFO:
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::info,
                                        "[#severity] (#tag) #message");
    break;
  case LogLevel::WARNING:
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::warning,
                                        "[#severity] (#tag) #message");
    break;
  case LogLevel::ERROR:
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::error,
                                        "[#severity] (#tag) #message");
    break;
  case LogLevel::FATAL:
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::fatal,
                                        "[#severity] (#tag) #message");
    break;
  default:
    std::cerr << "Internal error : Unable to map log level to logger"
              << std::endl;
    std::exit(1);
    break;
  };
}

int show_usage(std::string_view program_name)
{
  std::cout << program_name << std::endl;
  std::cout << "\nUsage:\n";
  std::cout << "  " << program_name
            << " [options] [<include-directories>] [<source-file>]"
            << std::endl;
  std::cout << "\nOptions:\n";
  std::cout << "  -h --help             Show this help screen\n";
  std::cout << "  -a --analyze          Analyze input before execution\n";
  std::cout << "  -n --norun            Disable execution\n";
  std::cout << "  -i --include          Include a ':' delimited directory list\n";
  std::cout << "  -l --log <level>      Set logging level\n";
  std::cout << "\n     Levels:\n";

  for (auto &i : logger_args) {
    std::cout << "            " << i.first << std::endl;
  }

  std::cout << "\nNeglecting to pass in a source file will start titan in REPL mode"
            << std::endl;
  return 0;
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
  Builds the list of include directories from -i or --include and ensures
  that each item given is a directory
*/
std::vector<std::string> parse_includes(std::string includes)
{
  std::vector<std::string> include_directories;
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
  return include_directories;
}

int main(int argc, char **argv)
{
  logger_args["trace"] = LogLevel::TRACE;
  logger_args["debug"] = LogLevel::DEBUG;
  logger_args["info"] = LogLevel::INFO;
  logger_args["warn"] = LogLevel::WARNING;
  logger_args["error"] = LogLevel::ERROR;
  logger_args["fatal"] = LogLevel::FATAL;
  logger_level = logger_args["error"];

  std::vector<std::string> arguments(argv, argv + argc);

  bool analyze = false;
  bool execute = true;
  std::string_view program_name = arguments[0];
  std::vector<std::string> include_dirs;
  std::string file;

  for (size_t idx = 1; idx < arguments.size(); ++idx) {

    auto &arg = arguments[idx];

    if (arg == "-h" || arg == "--help") {
      return show_usage(program_name);
    }

    if (arg == "-a" || arg == "--analyze") {
      analyze = true;
      continue;
    }

    if (arg == "-n" || arg == "--norun") {
      execute = false;
      continue;
    }

    if (arg == "-l" || arg == "--log") {
      if (arguments.size() <= idx + 1) {
        std::cout << "No value given to \"" << arg << "\"" << std::endl;
        std::exit(0);
      }
      set_logger_level(arguments[idx + 1]);
      idx += 1;
      continue;
    }

    if (arg == "-i" || arg == "--include") {
      if (arguments.size() <= idx + 1) {
        std::cout << "No value given to \"" << arg << "\"" << std::endl;
        std::exit(0);
      }
      include_dirs = parse_includes(arguments[idx + 1]);
      idx += 1;
      continue;
    }

    if (file.empty()) {
      file = arg;
    }
    else {
      std::cout << "Multiple source files given. First given \"" << file
                << "\" and then given \"" << arg << "\"\n"
                << "Please provide only a single file" << std::endl;
      std::exit(1);
    }
  }

  setup_logger();

  if (!analyze && !execute) {
    std::cout << "Nothing to do" << std::endl;
    return 0;
  }

  titan::titan t;
  t.set_analyze(analyze);
  t.set_execute(execute);

  if (file.empty()) {
    return t.do_repl();
  }

  return t.do_run(file);
}
