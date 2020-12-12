#define _GNU_SOURCE

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw3.h"

#define INCLUSIVE 0     // []
#define EXCLUSIVE 1     // [^]

/**
 * print the error to stderr
 */
void error_log(const char *format, ...)
{
   char buffer[256];
   va_list args;
   va_start(args, format);
   vsprintf(buffer, format, args);
   va_end(args);
   fprintf(stderr, "ERROR: %s\n", buffer);
   exit(EXIT_FAILURE);
}

/**
 * log the error
 * for debug use only
 */
void debug_log(const char *format, ...)
{
#ifdef DEBUG_MODE
   va_list args;
   va_start(args, format);
   vprintf(format, args);
   va_end(args);
   printf("\n");
#endif
}

/**
 * delete a given RegexItem
 * free all the memory
 */
void DeleteRegexItem(RegexItem *item)
{
   switch (item->category)
   {
   case C_GROUP_INCLUSIVE:  // []
   case C_GROUP_EXCLUSIVE:  // [^]
   case C_GROUP_CAPTURING:
      DELARR(item->u.items, free);
   case C_SINGLE:           // char
   case C_CLASS:
   case C_BEGINNING:        // ^
   case C_END:              // $
      free(item);
   }
}

/**
 * Try to match a RegexItem with a single character
 * RegexItem *item: RegexItem to be matched
 * char c: the character to match
 * returns 1 if matched; 0 otherwise
 */
int try_match_single_character(RegexItem *item, char c)
{
   if (item->category == C_SINGLE)  // if the RegexItem should be a defined char
   {
      return item->u.c == c;
   }
   else if (item->category == C_CLASS)
   {
      switch (item->u.mode)
      {
      case CLASS_ANY:               // .
         return 1;
      case CLASS_DIGIT:             // \d
         return isdigit(c);
      case CLASS_NDIGIT:            // \D
         return !isdigit(c);
      case CLASS_LETTER:            // \w
         return isalpha(c);
      case CLASS_NLETTER:           // \W
         return !isalpha(c);
      case CLASS_WHITESPACE:        // \s
         return isspace(c);
      case CLASS_NWHITESPACE:       // \S
         return !isspace(c);
      }
   }
   else                             // wrong classification
      ERROR_LOG("This method only work with simple regex item categories.")
   return 0;
}

/**
 * Try to match a char with a regex group
 * RegexItem *item: RegexItem to be matched
 * char c: the character to match
 * returns 1 if matched; 0 otherwise
 */
int try_match_group_character(RegexItem *item, char c)
{
   if (item->category != C_GROUP_EXCLUSIVE && item->category != C_GROUP_INCLUSIVE)
      ERROR_LOG("This method only work with character groups")  // wrong classification

   Array *items = item->u.items;    // get the item in the group
   for (int itemIndex = 0; itemIndex < items->length; itemIndex++)
   {
      RegexItem *subItem = items->Get(items, itemIndex);        // iterate thru items
      int matched = try_match_single_character(subItem, c);     // see if there's matched char
      if (matched)
         return item->category == C_GROUP_INCLUSIVE;            // decide the group type and result
   }
   return item->category == C_GROUP_EXCLUSIVE;
}

/**
 * Try to match a string with a regex group
 * RegexItem *item: RegexItem to be matched
 * const char *str: string to match
 * int index: index of the string
 * returns 1 if matched; 0 otherwise
 */
int try_match(RegexItem *item, const char *str, int index)
{
   if (index >= strlen(str)) // '=' is to avoid '\0' will be matched
      return 0;

   if (item->category == C_SINGLE || item->category == C_CLASS)
      return try_match_single_character(item, str[index]);      // match a character
   else if (item->category == C_GROUP_EXCLUSIVE || item->category == C_GROUP_INCLUSIVE)
      return try_match_group_character(item, str[index]);       // match a group
   else if (item->category == C_GROUP_CAPTURING)                // wrong classification
      ERROR_LOG("matching for capturing group hasn't been supported.")
   else
      ERROR_LOG("unknown regex item category: %d", item->category)
   return 0;
}

/**
 * Create a new RegexState
 * int strIndex: string index
 * int regexIndex: RegexItem array index
 * returns: pointer of the RegexState
 */
RegexState *NewRegexState(int strIndex, int regexIndex)
{
   RegexState *state = calloc(1, sizeof(RegexState));   // memory alloc
   state->strIndex = strIndex;                                 // set up properties
   state->regexIndex = regexIndex;
   state->occurrence = 0;
   state->start = -1;
   state->end = -1;
   state->last = 1;
   return state;
}

/**
 * Copy a RegexState
 * RegexState* other: the RegexState to copy
 * returns: pointer of the new RegexState
 */
RegexState *CopyRegexState(RegexState* other) {
   RegexState *state = calloc(1, sizeof(RegexState));   // alloc a new state instance and copy everything
   state->strIndex = other->strIndex;
   state->regexIndex = other->regexIndex;
   state->occurrence = other->occurrence;
   state->start = other->start;
   state->end = other->end;
   state->iter = other->iter;
   other->last = 0;
   state->last = 1;
   return state;
}

/**
 * Delete a RegexState instance
 * RegexState* *state: the RegexState to delete
 */
void DeleteRegexState(RegexState *state)
{
   if (state->last && state->iter != NULL) {
      DeleteRegexIter(state->iter);
   }
   free(state);
}

/**
 * Match regex pattern with a line
 * RegexIter *iter: RegexItem Array iterator
 */
int regex_line_match(RegexIter *iter)
{
   const char *str = iter->str;         // get the string to match
   Array *regexArr = iter->regexArray;  // get the RegexItem ARRAY

   Array *stack = iter->stack;          // RegexState stack
   Array *ongoing = iter->ongoing;

   if (regexArr->length == 0)
      return -1;
   int len = strlen(str);
   int regexLen = regexArr->length;

   while (stack->length > 0)            // iterate thru the stack
   {
      RegexState *state = (RegexState *)stack->Pop(stack);  // get the state
      while (ongoing->length > 0)       // match the ongoing pattern first
      {
         RegexState *last = ongoing->Last(ongoing);
         if (last->regexIndex > state->regexIndex)  // if already went through this regex pattern
         {
            ongoing->Pop(ongoing);                  // move and free
            DeleteRegexState(last);
            continue;
         }
         else if (last->regexIndex == state->regexIndex)
         {
            if (last->occurrence >= state->occurrence)  // if already reaches the occurrence number
            {
               ongoing->Pop(ongoing);                   // go to next ongoing state
               DeleteRegexState(last);                  // free the last state
               continue;
            }
         }
         break;
      }
      ongoing->Append(ongoing, state);
      DEBUG_LOG("ongoing length: %d", ongoing->length);

      int regexIndex = state->regexIndex;   // update the index
      int strIndex = state->strIndex;

      if (regexIndex == regexLen)
      {
         // find a match.
         iter->end = strIndex;
         return 1;
      }
      // get RegexItem
      RegexItem *item = (RegexItem *)regexArr->Get(regexArr, regexIndex);
      DEBUG_LOG("working on regex #%d(%d-%d), position %d", regexIndex, item->repeatMin, item->repeatMax, strIndex)

      if (NULL != state->iter) // drain all the variants
      {
         RegexIter *groupIter = state->iter;
         DEBUG_LOG("this is a state of group #%d, occurrence: %d", item->groupIndex, state->occurrence);

         if (groupIter->next(groupIter)) // show its variants.
         {
            state->start = groupIter->start;
            state->end = groupIter->end;

            DEBUG_LOG("[push] push back");
            stack->Append(stack, CopyRegexState(state)); // push back the state since it still has more variants.

            DEBUG_LOG("occurrence: %d, from %d to %d", state->occurrence, groupIter->start, groupIter->end)

            if (state->occurrence >= item->repeatMin)
            {
               DEBUG_LOG("[push] move to the next regex item: %d, strIndex: %d", regexIndex + 1, groupIter->end);
               stack->Append(stack, NewRegexState(groupIter->end, regexIndex + 1));
            }

            // try growing the occurrence of a state while it's less than max occurences.
            if (state->occurrence < item->repeatMax)
            {
               DEBUG_LOG("[push] state of group $%d tries to grow to %d", item->groupIndex, state->occurrence + 1);
               RegexState *newState = NewRegexState(groupIter->end, regexIndex);
               newState->iter = NewRegexIter(str, item->u.items, groupIter->end);
               newState->occurrence = state->occurrence + 1;
               stack->Append(stack, newState);
            }
         }
         else
         {
            state = ongoing->Pop(ongoing);
            DeleteRegexState(state);
         }
         continue;
      }
      // handle different regex patterns
      if (item->category == C_BEGINNING)    // ^
      {
         if (strIndex == 0)
            stack->Append(stack, NewRegexState(strIndex, regexIndex + 1));
         continue;
      }

      if (item->category == C_END)         // $
      {
         if (strIndex == len)
            stack->Append(stack, NewRegexState(strIndex, regexIndex + 1));
         continue;
      }

      if (item->category == C_GROUP_CAPTURING)  // ()
      {
         if (item->repeatMin == 0) // simply skip the capturing group when the occurrence of it can be 0
         {
            RegexState *state = NewRegexState(strIndex, regexIndex + 1);
            stack->Append(stack, state);
         }

         if (item->repeatMax < item->repeatMin || item->repeatMax == 0)
            ERROR_LOG("invalid occurrence range [%d, %d]", item->repeatMin, item->repeatMax)

         DEBUG_LOG("[push] adding capturing group matching...")
         RegexState *newState = NewRegexState(strIndex, regexIndex);    // init a new RegexState
         newState->iter = NewRegexIter(str, item->u.items, strIndex);
         newState->occurrence = 1;                                      // set default occurrence
         stack->Append(stack, newState);                                // append it to the stack
      }
      else                  // handle all other patterns
      {
         for (int occurence = 0; occurence <= item->repeatMax; occurence++)
         {
            if (strIndex + occurence > len)     // if it is impossible to occure
               break;

            if (occurence >= item->repeatMin) // it's a valid state and we can put it into stack.
            {
               DEBUG_LOG("occurrence: %d, at position %d", occurence, strIndex + occurence)
               stack->Append(stack, NewRegexState(strIndex + occurence, regexIndex + 1));
            }
            else
               DEBUG_LOG("occurrence: %d, skipped", occurence)

            if (!try_match(item, str, strIndex + occurence))    // if failed to match
               break;
         }
      }
   }
   return 0;
}

/**
 * Init a new RegexIter
 * const char *str: string
 * Array *regexArr: regex array
 * int start: index
 * returns: RegexIter pointer
 */
RegexIter *NewRegexIter(const char *str, Array *regexArr, int start)
{
   RegexIter *iter = calloc(1, sizeof(RegexIter));  // memory alloc
   iter->str = str;                                        // init all properties
   iter->start = start;
   iter->end = start;
   iter->regexArray = regexArr;
   iter->stack = NewArray();
   iter->ongoing = NewArray();

   if (regexArr->length > 0)
      iter->stack->Append(iter->stack, NewRegexState(start, 0));

   iter->next = regex_line_match;   // set the funcion
   return iter;
}

/**
 * Delete a RegexIter
 * RegexIter *iter: RegexIter to delete
 */
void DeleteRegexIter(RegexIter *iter)
{
   DEBUG_LOG("delete iter stack: %d", iter->stack->length);
   DELARR(iter->stack, DeleteRegexState);
   DEBUG_LOG("delete iter ongoing: %d", iter->ongoing->length);
   DELARR(iter->ongoing, DeleteRegexState);
   free(iter);
}

/**
 * Build the RegexItem array based on given string
 * const char *regex: string input
 * returns: pointer of the array
 */
Array *build_regex_array(const char *regex)
{
   int index = 0;
   return parse_regex_array(regex, &index, 0);
}

/**
 * Read the regex file
 * const char *filename: regex file name
 * char *regex: string to store the regex string
 */
void read_regex_file(const char *filename, char *regex)
{
   FILE *fp;
   fp = fopen(filename, "r");
   if (fp == NULL)
      ERROR_LOG("failed to open regex file %s", filename)
   fscanf(fp, "%s", regex);     // read the file into the string
   fclose(fp);
}

/**
 * Trim a given string char *str
 */
void trim_end(char *str)
{
   for (int index = strlen(str) - 1; index >= 0; index--)
   {
      if (str[index] == '\n' || str[index] == '\r')
         str[index] = '\0';
      else
         break;
   }
}

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
* the groups arrays is dynamically allocated in the regex_match() function
* only if necessary; it contains each group identified by and captured
* within parentheses, with the first match at index [1], the second
* match at [2], etc. (therefore, index [0] is always a NULL pointer)
*
* the captured_groups integer corresponds to the groups array above;
* this value specifies how many groups were identified (e.g., 2)
*
* (v1.2) if multiple lines match, both groups and captured_groups
* correspond to the last line matched
*
* regex_match() returns the number of lines that matched (zero or more)
* or -1 if an error occurred
*/
int regex_match(const char *filename, const char *regex,
                char ***matches, int trim_to_match,
                char ***groups, int *captured_groups)
{
   FILE *fp;
   char *line = NULL;
   size_t len = 0;
   size_t read;

   fp = fopen(filename, "r");   // read input file
   if (fp == NULL)
      ERROR_LOG("failed to open file %s", filename);

   Array *regexArr = build_regex_array(regex);  // build regex array
   Array *matchesArr = NewArray();              // build matches array

   RegexState *state = NULL;
   Array *groupArr = NewArray();
   *captured_groups = 0;

   while ((read = getline(&line, &len, fp)) != -1)
   {
      trim_end(line);   // remove the end of line
      for (int start = 0; start < strlen(line); start++)
      {
         RegexIter *iter = NewRegexIter(line, regexArr, start);
         if (iter->next(iter))
         {
            char *match;
            if (trim_to_match)  // if result should be trimmed
            {
               match = calloc(1, iter->end - iter->start);
               strncpy(match, line + iter->start, iter->end - iter->start);
            }
            else
            {
               match = calloc(1, strlen(line) + 1);
               strncpy(match, line, strlen(line));
            }
            matchesArr->Append(matchesArr, match);

            // only the last group matches should be preserved.
            groupArr->Reset(groupArr, free);
            groupArr->Append(groupArr, NULL); // the first value of matching groups should be NULL

            for (int i = 0; i < iter->ongoing->length; i++)
            {
               state = iter->ongoing->Get(iter->ongoing, i);
               DEBUG_LOG("regexIndex %d, occurrence %d", state->regexIndex, state->occurrence);
               if (state->end > state->start) {
                  char *buf = calloc(1, state->end - state->start + 1);
                  strncpy(buf, line + state->start, state->end - state->start);
                  groupArr->Append(groupArr, buf);
               }
            }

            start = strlen(line); // a simple trick to quit loop
         }
         DeleteRegexIter(iter);
      }
   }

   // clean up and free all memories
   if (NULL != line)
      free(line);
   fclose(fp);

   int result = matchesArr->length;
   *matches = (char **)matchesArr->items;
   free(matchesArr);

   if (groupArr->length > 0)
   {
      *captured_groups = groupArr->length - 1;
      *groups = (char **)groupArr->items;
      free(groupArr);
   }
   else
      DELARR(groupArr, free);

   DELARR(regexArr, DeleteRegexItem);
   return result;
}

#ifdef USE_MY_MAIN_FUNCTION
int main(int argc, char *argv[])
{
   char regex[512];
   char **matches;
   char **groups;
   int captured_groups;

   if (argc < 3)    // check arguments
      ERROR_LOG("Invalid arguments")
   char *regex_file = argv[1];
   char *text_file = argv[2];

   read_regex_file(regex_file, regex);  // read the regex file
   int count = regex_match(text_file, regex, &matches, 0, &groups, &captured_groups);   // count total matches
   for (int i = 0; i < count; i++)      // print matches
   {
      printf("matches: %s\n", matches[i]);
      free(matches[i]);
   }
   printf("last captured groups [%d]:\n", captured_groups);

   for (int i = 0; i < captured_groups; i++)    // print captured group
   {
      printf("%d. %s\n", i, groups[i + 1]);
      free(groups[i + 1]);
   }

   if (captured_groups)
      free(groups);
   free(matches);
   return 0;
}
#endif