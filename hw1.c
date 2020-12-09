#define _GNU_SOURCE

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw1.h"

#define INCLUSIVE 0
#define EXCLUSIVE 1

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

void DeleteRegexItem(RegexItem *item)
{
   switch (item->category)
   {
   case C_GROUP_INCLUSIVE:
   case C_GROUP_EXCLUSIVE:
   case C_GROUP_CAPTURING:
      DeleteArray(item->u.items);
   case C_SINGLE:
   case C_CLASS:
      free(item);
   }
}

int try_match_single_character(RegexItem *item, char c)
{
   if (item->category == C_SINGLE)
   {
      return item->u.c == c;
   }
   else if (item->category == C_CLASS)
   {
      switch (item->u.mode)
      {
      case CLASS_ANY:
         return 1;
      case CLASS_DIGIT:
         return isdigit(c);
      case CLASS_NDIGIT:
         return !isdigit(c);
      case CLASS_LETTER:
         return isalpha(c);
      case CLASS_NLETTER:
         return !isalpha(c);
      case CLASS_WHITESPACE:
         return isspace(c);
      case CLASS_NWHITESPACE:
         return !isspace(c);
      }
   }
   else
      ERROR_LOG("This method only work with simple regex item categories.")
   return 0;
}

int try_match_group_character(RegexItem *item, char c)
{
   if (item->category != C_GROUP_EXCLUSIVE && item->category != C_GROUP_INCLUSIVE)
      ERROR_LOG("This method only work with character groups")

   Array *items = item->u.items;
   for (int itemIndex = 0; itemIndex < items->length; itemIndex++)
   {
      RegexItem *subItem = items->Get(items, itemIndex);
      int matched = try_match_single_character(subItem, c);
      if (matched)
         return item->category == C_GROUP_INCLUSIVE;
   }
   return item->category == C_GROUP_EXCLUSIVE;
}

int try_match(RegexItem *item, const char *str, int index)
{
   if (index >= strlen(str)) // '=' is to avoid '\0' will be matched
      return 0;

   if (item->category == C_SINGLE || item->category == C_CLASS)
      return try_match_single_character(item, str[index]);
   else if (item->category == C_GROUP_EXCLUSIVE || item->category == C_GROUP_INCLUSIVE)
      return try_match_group_character(item, str[index]);
   else if (item->category == C_GROUP_CAPTURING)
      ERROR_LOG("matching for capturing group hasn't been supported.")
   else
      ERROR_LOG("unknown regex item category: %d", item->category)
   return 0;
}

RegexState *NewRegexState(int strIndex, int regexIndex, RegexState *prev)
{
   RegexState *state = calloc(1, sizeof(RegexState));
   state->strIndex = strIndex;
   state->regexIndex = regexIndex;
   state->start = -1;
   state->end = -1;
   state->occurence = 0;
   state->prev = prev;
   return state;
}

void DeleteState(RegexState *state)
{
   free(state);
   if (NULL != state->iter)
      DeleteRegexIter(state->iter);
}

int regex_line_match(RegexIter *iter)
{
   const char *str = iter->str;
   Array *regexArr = iter->regexArray;
   Array *stack = iter->stack;

   if (regexArr->length == 0)
      return -1;
   int len = strlen(str);
   int regexLen = regexArr->length;

   while (stack->length > 0)
   {
      RegexState *state = (RegexState *)stack->Pop(stack);
      DEBUG_LOG("[pop %d]", stack->length);
      int regexIndex = state->regexIndex;
      int strIndex = state->strIndex;

      if (regexIndex == regexLen)
      {
         // find a match.
         iter->end = strIndex;
         iter->current = state;
         return 1;
      }

      RegexItem *item = (RegexItem *)regexArr->Get(regexArr, regexIndex);
      DEBUG_LOG("working on regex #%d(%d-%d), position %d", regexIndex, item->repeatMin, item->repeatMax, strIndex)

      if (NULL != state->iter) // drain all the variants
      {
         RegexIter *groupIter = state->iter;
         int occurence = state->occurence;
         DEBUG_LOG("this is a state of group #%d, occurence: %d", item->groupIndex, occurence);

         if (groupIter->next(groupIter)) // show its variants.
         {
            state->start = groupIter->start;
            state->end = groupIter->end;

            DEBUG_LOG("[push] push back");
            stack->Append(stack, state); // push back the state since it still has more variants.

            DEBUG_LOG("occurence: %d, from %d to %d", occurence, groupIter->start, groupIter->end)

            if (occurence >= item->repeatMin)
            {
               DEBUG_LOG("[push %d] move to the next regex item: %d, strIndex: %d", stack->length, regexIndex + 1, groupIter->end);
               stack->Append(stack, NewRegexState(groupIter->end, regexIndex + 1, state));
            }

            // try growing the occurence of a state while it's less than max occurences.
            if (occurence < item->repeatMax)
            {
               DEBUG_LOG("[push] state of group $%d tries to grow to %d", item->groupIndex, occurence + 1);
               RegexState *newState = NewRegexState(groupIter->end, regexIndex, state);
               newState->iter = NewRegexIter(str, item->u.items, groupIter->end);
               newState->occurence = occurence + 1;
               stack->Append(stack, newState);
            }
         }
         else
         {
            state->start = -1;
            state->end = -1;
            DeleteRegexIter(state->iter);
         }
         continue;
      }

      if (item->category == C_BEGINNING)
      {
         if (strIndex == 0)
            stack->Append(stack, NewRegexState(strIndex, regexIndex + 1, state));
         continue;
      }

      if (item->category == C_END)
      {
         if (strIndex == len)
            stack->Append(stack, NewRegexState(strIndex, regexIndex + 1, state));
         continue;
      }

      if (item->category == C_GROUP_CAPTURING)
      {
         if (item->repeatMin == 0) // simply skip the capturing group when the occurence of it can be 0
         {
            RegexState *state = NewRegexState(strIndex, regexIndex + 1, state);
            stack->Append(stack, state);
         }

         if (item->repeatMax < item->repeatMin || item->repeatMax == 0)
            ERROR_LOG("invalid occurence range [%d, %d]", item->repeatMin, item->repeatMax)

         DEBUG_LOG("[push] adding capturing group matching...")
         RegexState *newState = NewRegexState(strIndex, regexIndex, state);
         newState->iter = NewRegexIter(str, item->u.items, strIndex);
         newState->occurence = 1;
         stack->Append(stack, newState);
      }
      else
      {
         for (int occurence = 0; occurence <= item->repeatMax; occurence++)
         {
            if (strIndex + occurence > len)
               break;

            if (occurence >= item->repeatMin) // it's a valid state and we can put it into stack.
            {
               DEBUG_LOG("occurence: %d, at position %d", occurence, strIndex + occurence)
               stack->Append(stack, NewRegexState(strIndex + occurence, regexIndex + 1, state));
            }
            else
               DEBUG_LOG("occurence: %d, skipped", occurence)

            if (!try_match(item, str, strIndex + occurence))
               break;
         }
      }
   }
   return 0;
}

RegexIter *NewRegexIter(const char *str, Array *regexArr, int start)
{
   RegexIter *iter = calloc(1, sizeof(RegexIter));
   iter->str = str;
   iter->start = start;
   iter->end = start;
   iter->regexArray = regexArr;
   iter->stack = NewArray();

   if (regexArr->length > 0)
      iter->stack->Append(iter->stack, NewRegexState(start, 0, NULL));

   iter->next = regex_line_match;
   return iter;
}

void DeleteRegexIter(RegexIter *iter)
{
   DeleteArray(iter->stack);
   free(iter);
}

Array *build_regex_array(const char *regex)
{
   int index = 0;
   return parse_regex_array(regex, &index, 0);
}

void read_regex(const char *filename, char *regex)
{
   FILE *fp;
   fp = fopen(filename, "r");
   if (fp == NULL)
      ERROR_LOG("failed to open regex file %s", filename)
   fscanf(fp, "%s", regex);
}

int regex_match(const char *filename, const char *regex,
                char ***matches, int trim_to_match,
                char ***groups, int *captured_groups)
{
   FILE *fp;
   char *line = NULL;
   size_t len = 0;
   size_t read;

   fp = fopen(filename, "r");
   if (fp == NULL)
      ERROR_LOG("failed to open file %s", filename);

   Array *regexArr = build_regex_array(regex);

   while ((read = getline(&line, &len, fp)) != -1)
   {
      for (int start = 0; start < strlen(line); start++)
      {
         RegexIter *iter = NewRegexIter(line, regexArr, start);
         if (iter->next(iter))
         {
            char *match = calloc(1, iter->end - iter->start);
            strncpy(match, line + iter->start, iter->end - iter->start);
            printf("match: %s\ngroups:\n", match);

            RegexState *state = iter->current;
            while (state != NULL)
            {
               if (state->start >= 0)
               {
                  char *buf = calloc(1, state->end - state->start);
                  strncpy(buf, line + state->start, state->end - state->start);
                  printf("%s\n", buf);
               }
               state = state->prev;
            }
            DeleteRegexIter(iter);
            break;
         }
         DeleteRegexIter(iter);
      }
   }

   DeleteArray(regexArr);
   return 0;
}

int main(int argc, char *argv[])
{
   char regex[512];
   char **matches;
   char **groups;
   int captured_groups;

   if (argc < 3)
      ERROR_LOG("Invalid arguments")
   char *regex_file = argv[1];
   char *text_file = argv[2];

   read_regex(regex_file, regex);
   int count = regex_match(text_file, regex, &matches, 1, &groups, &captured_groups);
   for (int i = 0; i < count; i++)
   {
      printf("%s\n", matches[i]);
   }
   return 0;
}

// test("123456789", "\\d(\\d{3,5})+\\d$");
// test("(0991)484-3933", "\\((\\d++)\\)(\\d+){2}-(\\d+)");