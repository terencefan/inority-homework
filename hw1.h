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

#define INF ((1 << 30) - 1 + (1 << 30))

#define C_SINGLE 1          // a single character, like 'a', '0', '\\', '<', '['
#define C_CLASS 2           // character class, like '.', '\\', '\w', '\d'
#define C_BEGINNING 4       // beginning, ^
#define C_END 5             // end, $
#define C_GROUP_CAPTURING 6 // a capturing group, like (\w+), the outer most regex will always be a capture group.
#define C_GROUP_INCLUSIVE 7 // character group in inclusive mode, like '[abc]'
#define C_GROUP_EXCLUSIVE 8 // character group in exclusive mode, like '[^abc]'

#define CLASS_ANY 1         // .
#define CLASS_DIGIT 2       // \d
#define CLASS_NDIGIT 3      // \D
#define CLASS_LETTER 4      // \w
#define CLASS_NLETTER 5     // \W
#define CLASS_WHITESPACE 6  // \s
#define CLASS_NWHITESPACE 7 // \S

typedef struct
{
    int category;

    int repeatMin;
    int repeatMax;

    union
    {
        char c;       // for a single character
        int mode;     // for character class
        Array *items; // for character/capturing group
    } u;

    int groupIndex;

} RegexItem;

typedef struct
{
    int regexIndex;
    int strIndex;
} RegexState;

RegexItem *
NewRegexItem(int category);

/**
 * log to stderr.
 * format is the string format that needs to be printed
 * can take as many arguments as needed
 */
void error_log(const char *format, ...);

/**
 * log to stdout if in debug mode.
 * format is the string format that needs to be printed
 * can take as many arguments as needed
 */
void debug_log(const char *format, ...);

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

int regex_match_line(const char *line, Array *regexArr, char **matched, char ***groups, int *captured_groups);

// parser.c

Array *parse_regex_array(const char *regex, int *index, int inGroup);

RegexItem *parse_escape_character(const char *regex, int *index);

RegexItem *parse_character_group(const char *regex, int *index);

RegexItem *parse_capturing_group(const char *regex, int *index);

RegexItem *parse_quantifiers(const char *regex, int *index, RegexItem *current);