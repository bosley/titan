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

#endif
