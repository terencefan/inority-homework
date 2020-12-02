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

RegexState *NewRegexState(int strIndex, int regexIndex, int start)
{
   RegexState *state = calloc(1, sizeof(RegexState));
   state->strIndex = strIndex;
   state->regexIndex = regexIndex;
   state->start = start;
   return state;
}

void DeleteState(RegexState *state)
{
   free(state);
   if (NULL != state->iter)
      DeleteRegexIter(state->iter);
}

int regex_line_match(RegexIter* iter)
{
   const char* str = iter->str;
   Array* regexArr = iter->regexArray;
   Array *stack = iter->stack;

   if (regexArr->length == 0)
      return -1;
   int len = strlen(str);
   int regexLen = regexArr->length;

   while (stack->length > 0)
   {
      RegexState *state = (RegexState*)stack->Pop(stack);
      DEBUG_LOG("[pop %d]", stack->length);
      int regexIndex = state->regexIndex;
      int strIndex = state->strIndex;
      int start = state->start;

      if (regexIndex == regexLen)
      {
         // find a match.
         iter->start = start;
         iter->end = strIndex;
         return 1;
      }

      RegexItem *item = (RegexItem *)regexArr->Get(regexArr, regexIndex);
      DEBUG_LOG("working on regex #%d(%d-%d), position %d", regexIndex, item->repeatMin, item->repeatMax, strIndex)

      if (NULL != state->iter) // we have to drain all the possibilities
      {
         RegexIter *groupIter = state->iter;
         int occurence = state->occurence;
         DEBUG_LOG("this is a state of group #%d, occurence: %d", item->groupIndex, occurence);

         if (groupIter->next(groupIter)) // show its variant
         {
            DEBUG_LOG("[push] push back");
            stack->Append(stack, state); // pushback the previous state.

            DEBUG_LOG("occurence: %d, from %d to %d", occurence + 1, groupIter->start, groupIter->end)
            if (occurence + 1 >= item->repeatMin)
            {
               DEBUG_LOG("[push %d] move to the next regex item: %d, strIndex: %d", stack->length, regexIndex + 1, groupIter->end);
               stack->Append(stack, NewRegexState(groupIter->end, regexIndex + 1, start));
            }

            if (occurence < item->repeatMax)
            {
               DEBUG_LOG("[push] state of group $%d tries to grow to %d", item->groupIndex, occurence + 1);
               // we have to try to growth the occurences of the capturing group.
               RegexState *newState = NewRegexState(groupIter->end, regexIndex, start);
               newState->iter = NewRegexIter(str, item->u.items, groupIter->end);
               newState->occurence = occurence + 1;
               stack->Append(stack, newState);
            }
         }
         else // the possibilities of this iterator has been drained.
         {
            DEBUG_LOG("cant find a match from position %d, occurence %d", strIndex, occurence)
            DeleteState(state);
         }
         continue;
      }
      DeleteState(state);

      if (item->category == C_BEGINNING)
      {
         if (strIndex == 0)
            stack->Append(stack, NewRegexState(strIndex, regexIndex + 1, start));
         continue;
      }

      if (item->category == C_END)
      {
         if (strIndex == len)
            stack->Append(stack, NewRegexState(strIndex, regexIndex + 1, start));
         continue;
      }

      if (item->category == C_GROUP_CAPTURING)
      {
         if (item->repeatMin == 0) // simply skip the capturing group when the occurence of it can be 0
         {
            RegexState *state = NewRegexState(strIndex, regexIndex + 1, start);
            stack->Append(stack, state);
         }

         DEBUG_LOG("adding capturing group matching...")
         RegexState *state = NewRegexState(strIndex, regexIndex, start);
         state->iter = NewRegexIter(str, item->u.items, strIndex);
         state->occurence = 0;
         stack->Append(stack, state);
      }
      else
      {
         int occurence = -1;
         do
         {
            occurence++;
            if (strIndex + occurence > len)
               break;

            if (occurence >= item->repeatMin) // it's a valid state and we can put it into stack.
            {
               DEBUG_LOG("occurence: %d, at position %d", occurence, strIndex + occurence)
               stack->Append(stack, NewRegexState(strIndex + occurence, regexIndex + 1, start));
            }
            else
               DEBUG_LOG("occurence: %d, skipped", occurence)
         } while (occurence < item->repeatMax && try_match(item, str, strIndex + occurence));
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
   {
      RegexItem *item = (RegexItem *)regexArr->Get(regexArr, 0);
      if (start >= 0) // specify the start position of matching process.
         iter->stack->Append(iter->stack, NewRegexState(start, 0, start));
      else if (item->category == C_BEGINNING) // a simple optimize.
         iter->stack->Append(iter->stack, NewRegexState(0, 0, 0));
      else
      {
         for (int i = strlen(str); i >= 0; i--)
            iter->stack->Append(iter->stack, NewRegexState(i, 0, i));
      }
   }

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

#ifdef HW1
void read_regex(const char *filename, char *regex)
{
   FILE *fp;
   fp = fopen(filename, "r");
   if (fp == NULL)
      ERROR_LOG("failed to open regex file %s", filename)
   fscanf(fp, "%s", regex);
}

int main(int argc, char *argv[])
{
   char regex[512];
   char **matches;

   if (argc < 3)
      ERROR_LOG("Invalid arguments")
   char *regex_file = argv[1];
   char *text_file = argv[2];

   read_regex(regex_file, regex);
   int count = regex_match(text_file, regex, &matches, 1);
   for (int i = 0; i < count; i++)
   {
      printf("%s\n", matches[i]);
   }
   return 0;
}
#endif

void test(const char *line, const char *regex)
{
   printf("%s, %s\n", line, regex);

   Array *regexArr = build_regex_array(regex);
   RegexIter *iter = NewRegexIter(line, regexArr, -1);
   while (iter->next(iter))
   {
      int start = iter->start, end = iter->end;
      char buf[256] = "";
      strncpy(buf, line + start, end - start);
      printf("--- match: %s, from %d to %d\n", buf, start, end);
   }
   DeleteRegexIter(iter);
   DeleteArray(regexArr);
}

#if !defined(HW1) && !defined(HW3)
int main()
{
   // test("abbbbbb", "a.*");
   // test("bbbbbb", "a.*");
   // test("bbbbbb", "a?.*");
   // test("cbaddcee", "a.*");
   // test("cbaddcee", "c.*d");
   // test("cbaddcee", "c.*c?e*");
   // test("cbaddee", "c.*c?e*");
   // test("cbaddee", "c.*ce*");

   // test("abbbbc", "b+");
   // test("abbbbc", "^b+");
   // test("abbbbc", "^ab+");
   // test("abbbbc", "b+$");
   // test("abbbbc", "b+c$");
   // test("abbbbc", "^ab+c$");

   // test("abcde", "a[bcd]+e");
   // test("abcde", "a[bc]+e");
   // test("abcde", "a[^e]+e");
   // test("abcde", "a[^de]+e");

   test("1-234-67", "^\\d-(\\d{2,3})+-\\d");
}
#endif