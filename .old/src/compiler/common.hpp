#ifndef COMPILER_COMMON_HPP
#define COMPILER_COMMON_HPP

#include <string>

namespace compiler {

struct SourceLine {
  std::string data;
  size_t file_line_no;
  std::string_view origin_file;
};

} // namespace compiler

#endif