#ifndef TITAN_ALERT_HPP
#define TITAN_ALERT_HPP

#include <string>

namespace alert {

enum class level { WARNING, ERROR };

//
//  Configuration for the "show" call. Mostly just to group data,
//  but comes with some methods to streamline usage
//
class config {
public:
  static constexpr size_t default_display_window_top = 5;
  static constexpr size_t default_display_window_bot = 5;

  config()
      : line(0), col(0), show_line_num(true), show_col_num(true),
        show_chunk(false), attn_at_line(false), attn_at_col(false),
        display_window_top(default_display_window_top),
        display_window_bot(default_display_window_bot)
  {
  }

  void set_basic(std::string in_file, std::string in_message, size_t in_line,
                 size_t in_col)
  {
    file = in_file;
    message = in_message;
    set_line_col(in_line, in_col);
  }

  void set_line_col(size_t in_line, size_t in_col)
  {
    line = in_line;
    col = in_col;
  }

  void set_window(size_t top, size_t bot)
  {
    display_window_top = top;
    display_window_bot = bot;
  }

  void set_show_chunk(bool value) { show_chunk = value; }

  void set_all_attn(bool value)
  {
    attn_at_line = value;
    attn_at_col = value;
  }

  size_t line;
  size_t col;
  bool show_line_num;
  bool show_col_num;
  bool show_chunk;
  bool attn_at_line;
  bool attn_at_col;
  size_t display_window_top;
  size_t display_window_bot;
  std::string file;
  std::string message;
};

extern void show(level lvl, const char *reporter, const config &cfg);

} // namespace alert

#endif
