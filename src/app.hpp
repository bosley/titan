#ifndef APP_DEFINITIONS_HPP
#define APP_DEFINITIONS_HPP

// Figure out what function macro to use    
#ifdef __GNUC__    
#define APP_FUNC __FUNCTION__    
#else    
#define APP_FUNC __func__    
#endif    
    
// Remove the leading path from the file name    
#define APP_FILE_NAME (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)    
    
#define APP_LINE __LINE__


#ifdef _WIN32

#define APP_COLOR_RED ""
#define APP_COLOR_GREEN ""
#define APP_COLOR_BLUE ""
#define APP_COLOR_CYAN ""
#define APP_COLOR_MAGENTA ""
#define APP_COLOR_YELLOW ""
#define APP_COLOR_END ""

#else

#define APP_COLOR_RED "\033[0;31m"
#define APP_COLOR_GREEN "\033[0;32m"
#define APP_COLOR_BLUE "\033[0;34m"
#define APP_COLOR_CYAN "\033[0;36m"
#define APP_COLOR_MAGENTA "\033[0;35m"
#define APP_COLOR_YELLOW "\033[0;33m"
#define APP_COLOR_END "\033[0m"

#endif


#endif
