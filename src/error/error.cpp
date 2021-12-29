#include "app.hpp"
#include "log/log.hpp"
#include "error.hpp"

#include <fstream>
#include <iostream>

namespace error
{

void error::display_error(const char* reporter, error::config cfg) 
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "] Error reported by " << reporter << std::endl;  

  std::cout << COLOR(red) << "<" << reporter << "> Error " << COLOR(none) << cfg.file;

  if(cfg.show_line_num) {
    std::cout << " line:" << cfg.line;
  }

  if(cfg.show_col_num) {
    std::cout << " col:" << cfg.line;
  }

  if(!cfg.show_chunk) {
    std::cout << cfg.message << std::endl;
    return;
  }

  std::ifstream target;
  target.open(cfg.file);

  if(!target.is_open()) {
    LOG(ERROR) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "] Unable to open " << cfg.file << std::endl;  

    std::cout << cfg.message << std::endl;
    return;
  }

  std::cout << std::endl;

  size_t line_no = 0;
  bool complete = false;
  bool add_line = cfg.line <= 5;
  std::vector<std::string> display;

  while(!complete) {

    line_no++;

    if(!add_line && line_no >= cfg.line - 5) {
      add_line = true;
    }

    if(line_no >= cfg.line + 5) {
      add_line = false;
      complete = true;
    }

    std::string line;

    if(!std::getline(target, line)) {
      complete = true;
    }

    if(add_line) {
      std::string display_line = {};

      // Check if we need to mark up the file
      //
      if(cfg.attn_at_line) {
        display_line += APP_COLOR_RED;
        display_line += std::to_string(line_no);
        display_line += APP_COLOR_END;
      }
      else {
        display_line += std::to_string(line_no);
      }
      display_line += "| " + line; 

      display.push_back(display_line);

      // Check for attn to tol

      if(cfg.attn_at_col) {
        std::string col_line = {};
        
        for(size_t idx = 0; idx < cfg.col; idx++) {
          col_line += APP_COLOR_GREEN;
          col_line += "~";
        }

        col_line += "^";

        col_line += APP_COLOR_END;

        display.push_back(col_line);
      }
    }
  }


  for(auto& d_line : display) {
    std::cout << d_line << std::endl;
  }

  std::cout << "Message : " << cfg.message << std::endl;

}

}
