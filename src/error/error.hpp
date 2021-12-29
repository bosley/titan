#ifndef COMPILER_ERROR_HPP
#define COMPILER_ERROR_HPP

#include <string>
namespace error 
{

class error
{
public:

  struct config {
    std::string file;
    std::string message;
    size_t line;
    size_t col;
    bool show_line_num;
    bool show_col_num;
    bool show_chunk;
    bool attn_at_line;
    bool attn_at_col;
  };

  void display_error(const char * reporter, config cfg);
};


}


#endif
