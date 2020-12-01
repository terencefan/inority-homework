#define _GNU_SOURCE

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw1.h"

#define CLASS_ANY 1         // .
#define CLASS_DIGIT 2       // \d
#define CLASS_NDIGIT 3      // \D
#define CLASS_LETTER 4      // \w
#define CLASS_NLETTER 5     // \W
#define CLASS_WHITESPACE 6  // \s
#define CLASS_NWHITESPACE 7 // \S

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

int get_character_class_mode(char c)
{
   switch (c)
   {
   case '.':
      return CLASS_ANY;
   case 'd':
      return CLASS_DIGIT;
   case 'D':
      return CLASS_NDIGIT;
   case 'w':
      return CLASS_LETTER;
   case 'W':
      return CLASS_NLETTER;
   case 's':
      return CLASS_WHITESPACE;
   case 'S':
      return CLASS_NWHITESPACE;
   default:
      ERROR_LOG("Invalid special character: \\%c", c)
   }
   return 0;
}

RegexItem *NewRegexItem(int category)
{
   RegexItem *item = calloc(1, sizeof(RegexItem));
   item->category = category;
   item->repeatMax = 1;
   item->repeatMin = 1;
   return item;
}

RegexItem *NewCharacterClass(char c)
{
   DEBUG_LOG("new character class \\%c", c);
   RegexItem *item = NewRegexItem(C_CLASS);
   item->u.mode = get_character_class_mode(c);
   return item;
}

RegexItem *NewCharactor(char c)
{
   DEBUG_LOG("new character %c", c);
   RegexItem *item = NewRegexItem(C_SINGLE);
   item->u.c = c;
   return item;
}

RegexItem *NewCharacterGroup()
{
   DEBUG_LOG("new character group");
   RegexItem *item = NewRegexItem(C_GROUP_INCLUSIVE);
   item->u.items = NewArray();
   return item;
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

int try_match_single_character(RegexItem *item, char c) {
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

int TryMatch(RegexItem *item, const char* str, int index)
{
   if (index >= strlen(str)) { // this can be somehow optimized.
      return -1;
   }

   if (item->category == C_SINGLE || item->category == C_CLASS)
   {
      return try_match_single_character(item, str[index]) ? index + 1 : -1;
   }
   else if (item->category == C_GROUP_EXCLUSIVE || item->category == C_GROUP_INCLUSIVE)
   {
      return try_match_group_character(item, str[index]) ? index + 1 : -1;
   }
   else if (item->category == C_GROUP_CAPTURING) 
      ERROR_LOG("matching for capturing group hasn't been supported.")
   else
      ERROR_LOG("unknown regex item category: %d", item->category)
   return 0;
}

RegexItem *parse_escape_character(const char *regex, int *index)
{
   if (regex[*index] != '\\' || (*index + 1) >= strlen(regex))
      ERROR_LOG("an escaped character should start with a '\\' and have at least length of 2")
   *index += 1; // move 1 step forward to skip '\\'

   char c = regex[*index];
   switch (c)
   {
   case '.':
   case '\\':
   case '^':
   case '$':
   case '?':
   case '+':
   case '*':
   case '{':
   case '}':
   case '[':
   case ']':
   case '(':
   case ')':
      return NewCharactor(c);
   default:
      return NewCharacterClass(regex[*index]);
   }
}

RegexItem *parse_character_group(const char *regex, int *index)
{
   if (regex[*index] != '[' || (*index + 1) >= strlen(regex))
      ERROR_LOG("character group should start with '[' and have at least length of 2")
   *index += 1; // move 1 step forward to skip '\\'

   RegexItem *group = NewCharacterGroup();
   RegexItem *temp;
   Array *items = group->u.items;

   if (regex[*index] == '^')
   {
      group->category = C_GROUP_EXCLUSIVE;
      *index += 1; // move 1 step forward to skip '^'
   }

   for (; *index < strlen(regex); (*index)++)
   {
      char c = regex[*index];
      switch (c)
      {
      case '[':
         break; // ignore
      case ']':
         DEBUG_LOG("end character group");
         return group;
      case '^':
         ERROR_LOG("^ could only present at the beginning of a character group.")
      case '.': // A single '.' is also a character class.
         temp = NewCharacterClass(c);
         items->Append(items, temp);
         break;
      case '\\':
         temp = parse_escape_character(regex, index);
         items->Append(items, temp);
         break;
      default:
         temp = NewCharactor(c);
         items->Append(items, temp);
         break;
      }
   }
   ERROR_LOG("character group should end with ']'")
   return NULL;
}

Array *build_regex_array(const char *regex)
{
   int index = 0;
   return parse_regex_array(regex, &index, 0);
}

Array *parse_regex_array(const char *regex, int *index, int inGroup)
{
   Array *regexArr = NewArray();
   int len = strlen(regex);

   if (*index < len)
   {
      switch (regex[*index])
      {
      case '+':
      case '?':
      case '*':
      case '{':
         ERROR_LOG("invalid regex string at position %d", *index)
         break;
      }
   }

   RegexItem *item;
   for (; *index < len; (*index)++)
   {
      char c = regex[*index];

      switch (c)
      {
      case '.':
         item = NewCharacterClass(c); // a single . is a special character class.
         regexArr->Append(regexArr, item);
         break;
      case '\\':
         item = parse_escape_character(regex, index);
         regexArr->Append(regexArr, item);
         break;
      case '(':
         if (inGroup)
            ERROR_LOG("capturing group cannot be nested.")
         item = parse_capturing_group(regex, index);
         break;
      case '[':
         item = parse_character_group(regex, index);
         regexArr->Append(regexArr, item);
         break;
      case '{':
      case '?':
      case '+':
      case '*':
         item = parse_quantifiers(regex, index, item);
         break;
      case ')':
         // indicate the ending of a capturing group.
         return regexArr;
      case ']':
      case '}':
         ERROR_LOG("invalid regex string at position %d", *index)
      case '^':
         if (*index != 0)
            ERROR_LOG("^ must present at the beginning of a valid regex string.")
         item = NewRegexItem(C_BEGINNING);
         regexArr->Append(regexArr, item);
         break;
      case '$':
         if (*index != len - 1)
            ERROR_LOG("$ must present at the end of a valid regex string.")
         item = NewRegexItem(C_END);
         regexArr->Append(regexArr, item);
         break;
      default:
         item = NewCharactor(c);
         regexArr->Append(regexArr, item);
         break;
      }
   }
   return regexArr;
}

int regex_line_match_backtrace(const char *str, Array *regexArr, int strIndex, int regexIndex, int occurence)
{
   if (regexIndex >= regexArr->length)
      return strIndex;

   RegexItem *item = (RegexItem *)regexArr->Get(regexArr, regexIndex);

   if (item->category == C_BEGINNING)
   {
      return strIndex == 0 ? regex_line_match_backtrace(str, regexArr, 0, regexIndex + 1, 0) : -1;
   }

   if (item->category == C_END)
   {
      return strIndex == strlen(str) ? strIndex : -1;
   }

   if (item->category == C_GROUP_CAPTURING)
      ERROR_LOG("this regex method cannot work with group capturing.")

   DEBUG_LOG("working on regex #%d(%d-%d), position %d, occurence %d", regexIndex, item->repeatMin, item->repeatMax, strIndex, occurence)

   if (occurence == item->repeatMax) { // move directly to the next regex item.
      return regex_line_match_backtrace(str, regexArr, strIndex, regexIndex + 1, 0);
   }

   if (strIndex < strlen(str)) // try consuming new characters only when current index smaller than the input str length.
   {
      int next = TryMatch(item, str, strIndex); // test if the str that starts from the current position matched the current regexItem.
      if (next >= 0)
      {
         int right = regex_line_match_backtrace(str, regexArr, next, regexIndex, occurence + 1); // greedy approach to try matching more occurences.
         if (right >= 0)
            return right;
      }
   }

   if (occurence >= item->repeatMin) { // move to the next regex item only if met the minimum occurence requirements.
      return regex_line_match_backtrace(str, regexArr, strIndex, regexIndex + 1, 0); 
   }
   return -1;
}

/**
 * Deprecated, use regex_match_line instead.
 */
int regex_line_match(const char *line, Array *regexArr, int left)
{
   return regex_line_match_backtrace(line, regexArr, left, 0, 0);
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
      int right = regex_line_match_backtrace(line, regexArr, left, 0, 0);
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
      printf("--- matches: %s\n", buf);
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

   test("abcde", "a[^e]{4,5}e");
}
#endif