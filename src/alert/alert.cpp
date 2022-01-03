#include "alert.hpp"
#include "app.hpp"
#include "log/log.hpp"

#include <fstream>
#include <iostream>
#include <string>

namespace alert {

static constexpr std::string_view line_break = "  _____________________";

void show(level lvl, const char *reporter, const config &cfg)
{
  LOG(DEBUG) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "] Alert reported by "
             << reporter << std::endl;

  std::string alert_type;
  switch (lvl) {
  case level::ERROR:
    std::cout << APP_COLOR_RED;
    break;
  case level::WARNING:
    std::cout << APP_COLOR_YELLOW;
    break;
  default:
    std::cout << APP_COLOR_GREEN;
    break;
  };

  std::cout << alert_type << " <" << reporter << "> " << APP_COLOR_END
            << cfg.file;

  if (cfg.show_line_num) {
    std::cout << " line:" << cfg.line;
  }

  if (cfg.show_col_num) {
    std::cout << " col:" << cfg.line;
  }

  if (!cfg.show_chunk) {
    if(!cfg.message.empty()) {
      std::cout << std::endl
                << APP_COLOR_CYAN << "\tMessage : " << APP_COLOR_END 
                << cfg.message;
    }
    std::cout << std::endl;
    return;
  }

  std::ifstream target;
  target.open(cfg.file);

  if (!target.is_open()) {
    LOG(ERROR) << TAG(APP_FILE_NAME) << "[" << APP_LINE << "] Unable to open "
               << cfg.file << std::endl;

    std::cout << cfg.message << std::endl;
    return;
  }

  std::cout << std::endl;

  size_t line_no = 0;
  bool complete = false;
  bool add_line = cfg.line <= cfg.display_window_bot;
  std::vector<std::string> display;

  if(!cfg.message.empty()) {
    std::cout << APP_COLOR_CYAN << "Message : " << APP_COLOR_END << cfg.message;
    std::cout << std::endl;
  }
  std::cout << line_break << std::endl;

  while (!complete) {

    line_no++;

    if (!add_line && line_no >= cfg.line - cfg.display_window_top) {
      add_line = true;
    }

    if (line_no >= cfg.line + cfg.display_window_bot) {
      add_line = false;
      complete = true;
    }

    std::string line;

    if (!std::getline(target, line)) {
      complete = true;
    }

    if (add_line) {
      std::string display_line = {};

      // Check if we need to mark up the file
      //
      if (cfg.attn_at_line && line_no == cfg.line) {
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

      if (cfg.attn_at_col && line_no == cfg.line) {
        std::string col_line = "   ";

        col_line += APP_COLOR_GREEN;

        for (size_t idx = 0; idx < cfg.col; idx++) {
          col_line += "~";
        }
        col_line += "^";

        col_line += APP_COLOR_END;

        display.push_back(col_line);
      }
    }
  }

  for (auto &d_line : display) {
    std::cout << d_line << std::endl;
  }

  std::cout << std::endl;
}

} // namespace alert
