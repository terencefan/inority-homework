#include "array.h"

#define ERROR_LOG(fmt, ...)            \
    do                                 \
    {                                  \
        error_log(fmt, ##__VA_ARGS__); \
    } while (0);
#define DEBUG_LOG(fmt, ...)            \
    do                                 \
    {                                  \
        debug_log(fmt, ##__VA_ARGS__); \
    } while (0);

void error_log(const char *format, ...);
void debug_log(const char *format, ...);

Array *build_regex_array(const char *regex);

int regex_line_match(const char *line, Array *regexArr, int left);