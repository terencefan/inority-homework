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

/**
 * log to stderr.
 * format is the string format that needs to be printed
 * can take as many arguments as needed
 */
void error_log(const char* format, ...);

/**
 * log to stdout if in debug mode.
 * format is the string format that needs to be printed
 * can take as many arguments as needed
 */
void debug_log(const char* format, ...);

/**
 * read the regex string and build an array of regex item
 * const char *regex: the regex string
 * returns: an array of regex items
 */
Array *build_regex_array(const char *regex);

/**
 * Deprecated, use regex_match_line instead.
 */
int regex_line_match(const char *line, Array *regexArr, int left);

int regex_match_line(const char *line, Array *regexArr, int* l, int* r);