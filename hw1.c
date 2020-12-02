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

RegexState *NewState(int strIndex, int regexIndex)
{
   RegexState *state = calloc(1, sizeof(RegexState));
   state->strIndex = strIndex;
   state->regexIndex = regexIndex;
   return state;
}

void DeleteState(RegexState *state)
{
   free(state);
}

Array *build_regex_array(const char *regex)
{
   int index = 0;
   return parse_regex_array(regex, &index, 0);
}

int regex_line_match(const char *str, Array *regexArr, int start)
{
   if (regexArr->length == 0)
      return -1;
   int len = strlen(str);
   int regexLen = regexArr->length;

   Array *stack = NewArray();
   stack->Append(stack, NewState(start, 0));

   int result = -1;

   while (stack->length > 0)
   {
      RegexState *state = (RegexState*)stack->Pop(stack);
      int regexIndex = state->regexIndex;
      int strIndex = state->strIndex;
      DeleteState(state);

      if (regexIndex == regexLen)
      {
         char buf[256] = "";
         strncpy(buf, str + start, strIndex - start);
         DEBUG_LOG("------ match: %s, from %d to %d", buf, start, strIndex);
         result = result < 0 ? strIndex : result;
         continue;
      }

      RegexItem *item = (RegexItem *)regexArr->Get(regexArr, regexIndex);
      DEBUG_LOG("working on regex #%d(%d-%d), position %d", regexIndex, item->repeatMin, item->repeatMax, strIndex)

      if (item->category == C_BEGINNING)
      {
         if (strIndex == 0)
            stack->Append(stack, NewState(strIndex, regexIndex + 1));
         continue;
      }

      if (item->category == C_END)
      {
         if (strIndex == len)
            stack->Append(stack, NewState(strIndex, regexIndex + 1));
         continue;
      }

      int occurence = -1;
      do
      {
         occurence++;
         if (strIndex + occurence > len)
            break;

         if (occurence >= item->repeatMin) // it's a valid state and we can put it into stack.
         {
            DEBUG_LOG("occurence: %d, at position %d", occurence, strIndex + occurence)
            stack->Append(stack, NewState(strIndex + occurence, regexIndex + 1));
         }
         else
         {
            DEBUG_LOG("occurence: %d, skipped", occurence)
         }
      } while (occurence < item->repeatMax && try_match(item, str, strIndex + occurence));
   }

   DeleteArray(stack);
   return result;
}

void read_regex(const char *filename, char *regex)
{
   FILE *fp;
   fp = fopen(filename, "r");
   if (fp == NULL)
      ERROR_LOG("failed to open regex file %s", filename)
   fscanf(fp, "%s", regex);
}

int regex_match(const char *filename, const char *regex, char ***matches, int trim_to_match)
{
   Array *regexArr = build_regex_array(regex);

   FILE *fp;
   char *line = NULL;
   size_t len = 0;
   size_t read;

   fp = fopen(filename, "r");
   if (fp == NULL)
      ERROR_LOG("failed to open file %s", filename)

   Array *matchArr = NewArray();
   int count = 0;
   while ((read = getline(&line, &len, fp)) != -1)
   {
      for (int left = 0; line[left] != '\0'; left++)
      {
         int right = regex_line_match(line, regexArr, left);
         if (right < 0)
            continue;

         if (trim_to_match)
         {
            char *buffer = malloc(right - left + 1);
            strncpy(buffer, line + left, right - left);
            buffer[right - left] = '\0';
            matchArr->Append(matchArr, buffer);
         }
         else
         {
            char *buffer = malloc(strlen(line + 1));
            strcpy(buffer, line);
            matchArr->Append(matchArr, buffer);
         }

         count += 1;
         break;
      }
   }

   if (line)
      free(line);
   fclose(fp);

   DeleteArray(regexArr);
   *matches = (char **)RemoveArray(matchArr);
   return count;
}

#ifdef HW1
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

int test_match(const char *line, const char *regex, int *l, int *r)
{
   Array *regexArr = build_regex_array(regex);
   for (int left = 0; line[left] != '\0'; left++)
   {
      int right = regex_line_match(line, regexArr, left);
      if (right >= 0)
      {
         *l = left;
         *r = right;
         DeleteArray(regexArr);
         return 1;
      }
   }
   DeleteArray(regexArr);
   return 0;
}

void test(const char *input, const char *regex)
{
   printf("%s, %s\n", input, regex);

   int l, r;
   if (test_match(input, regex, &l, &r))
   {
      char buf[256];
      strncpy(buf, input + l, r - l);
      printf("--- first match: %s\n", buf);
   }
   else
   {
      printf("--- match failed.\n");
   }
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

   test("abcddde", "^a[^e]{3,5}e$");
}
#endif