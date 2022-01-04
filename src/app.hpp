#ifndef APP_DEFINITIONS_HPP
#define APP_DEFINITIONS_HPP

// Figure out what function macro to use
#ifdef __GNUC__
#define APP_FUNC __FUNCTION__
#else
#define APP_FUNC __func__
#endif

// Remove the leading path from the file name
#define APP_FILE_NAME                                                          \
  (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1     \
                                    : __FILE__)

#define APP_LINE __LINE__

#ifdef _WIN32

#define APP_COLOR_BLACK ""
#define APP_COLOR_RED ""
#define APP_COLOR_GREEN ""
#define APP_COLOR_YELLOW ""
#define APP_COLOR_BLUE ""
#define APP_COLOR_MAGENTA ""
#define APP_COLOR_CYAN ""
#define APP_COLOR_WHITE ""

#define APP_COLOR_BRIGHT_BLACK ""
#define APP_COLOR_BRIGHT_RED ""
#define APP_COLOR_BRIGHT_GREEN ""
#define APP_COLOR_BRIGHT_YELLOW ""
#define APP_COLOR_BRIGHT_BLUE ""
#define APP_COLOR_BRIGHT_MAGENTA ""
#define APP_COLOR_BRIGHT_CYAN ""
#define APP_COLOR_BRIGHT_WHITE ""

#define APP_COLOR_END ""

#else

#define APP_COLOR_BLACK "\033[0;30m"
#define APP_COLOR_RED "\033[0;31m"
#define APP_COLOR_GREEN "\033[0;32m"
#define APP_COLOR_YELLOW "\033[0;33m"
#define APP_COLOR_BLUE "\033[0;34m"
#define APP_COLOR_MAGENTA "\033[0;35m"
#define APP_COLOR_CYAN "\033[0;36m"
#define APP_COLOR_WHITE "\033[0;37m"

#define APP_COLOR_BRIGHT_BLACK "\033[0;90m"
#define APP_COLOR_BRIGHT_RED "\033[0;91m"
#define APP_COLOR_BRIGHT_GREEN "\033[0;92m"
#define APP_COLOR_BRIGHT_YELLOW "\033[0;93m"
#define APP_COLOR_BRIGHT_BLUE "\033[0;94m"
#define APP_COLOR_BRIGHT_MAGENTA "\033[0;95m"
#define APP_COLOR_BRIGHT_CYAN "\033[0;96m"
#define APP_COLOR_BRIGHT_WHITE "\033[0;97m"

#define APP_COLOR_END "\033[0m"

#endif

#endif
