#include "array.h"

#define ERROR_LOG(fmt, ...) do { error_log(fmt, ##__VA_ARGS__); } while(0);
#define DEBUG_LOG(fmt, ...) do { debug_log(fmt, ##__VA_ARGS__); } while(0);

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

/** regex_match() applies the given regex to each line of the file specified
* by filename; all matching lines are stored in a dynamically allocated
* array called matches
*
* the matches array is dynamically allocated in the regex_match() function;
* therefore, it is up to the calling process to free the allocated memory
*
* trim_to_match is a flag that, when set, indicates that each line in the
* given file that matches the given regex must be trimmed to show only
* the matching substring
*
* regex_match() returns the number of lines that matched (zero or more)
* or -1 if an error occurred
*/
int regex_line_match(const char *line, Array *regexArr, int left);