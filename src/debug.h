#define COLOR_RED "\033[22;31m"
#define COLOR_YELLOW "\033[22;33m"
#define COLOR_GREEN "\033[22;32m"
#define COLOR_BLUE "\033[22;34m"
#define COLOR_END "\33[0m"
#ifndef DEBUG
#define debug(...)
#else
#define debug(debugstr, ...) \
    do{\
if(strlen(debugstr) > 2) {\
fprintf(stderr,"%-10s line: %-4d %20s: %s ",  __FILE__, __LINE__, __func__, COLOR_BLUE);\
}\
fprintf(stderr, debugstr"\n%s", ##__VA_ARGS__, COLOR_END);\
    } while(0)
#endif
