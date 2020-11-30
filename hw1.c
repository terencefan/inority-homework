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
      ERROR_LOG("Invalid special character: \\%c", c);
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

int TryMatch(RegexItem *item, char c)
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
   else if (item->category == C_GROUP_EXCLUSIVE || item->category == C_GROUP_INCLUSIVE)
   {
      Array *items = item->u.items;
      for (int itemIndex = 0; itemIndex < items->length; itemIndex++)
      {
         RegexItem *subItem = items->Get(items, itemIndex);
         int matched = TryMatch(subItem, c);
         if (matched)
            return item->category == C_GROUP_INCLUSIVE;
      }
      return item->category == C_GROUP_EXCLUSIVE;
   }
   else
      ERROR_LOG("unknown regex item category: %d", item->category);
   return 0;
}

RegexItem *parse_escape_character(const char *regex, int *index)
{
   if (regex[*index] != '\\' || (*index + 1) >= strlen(regex))
      ERROR_LOG("an escaped character should start with a '\\' and have at least length of 2");
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
      ERROR_LOG("character group should start with '[' and have at least length of 2");
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
         ERROR_LOG("^ could only present at the beginning of a character group.");
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
   ERROR_LOG("character group should end with ']'");
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
   RegexItem *current = NULL;

   if (strlen(regex) > 0)
   {
      if (regex[*index] == '+' || regex[*index] == '?' || regex[*index] == '*')
      {
         ERROR_LOG("regex string couldn't start with wildcard characters");
      }
   }

   int len = strlen(regex);
   int min, max;

   for (; *index < len; (*index)++)
   {
      char c = regex[*index];
      RegexItem *item = current;

      switch (c)
      {
      case '?':
         current->repeatMin = 0;
         current->repeatMax = 1;
         DEBUG_LOG("set wildcardType to ?");
         break;
      case '+':
         current->repeatMin = 1;
         current->repeatMax = INF;
         DEBUG_LOG("set wildcardType to +");
         break;
      case '*':
         current->repeatMin = 0;
         current->repeatMax = INF;
         DEBUG_LOG("set wildcardType to *");
         break;
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
         parse_quantifiers(regex, index, &min, &max);
         current->repeatMin = min;
         current->repeatMax = max;
         break;
      case ')':
         // indicate the ending of a capturing group.
         return regexArr;
      case ']':
      case '}':
         ERROR_LOG("invalid regex string at position %d", *index);
      case '^':
         if (*index != 0)
            ERROR_LOG("^ must present at the beginning of a valid regex string.");
         item = NewRegexItem(C_BEGINNING);
         regexArr->Append(regexArr, item);
         break;
      case '$':
         if (*index != len - 1)
            ERROR_LOG("$ must present at the end of a valid regex string.");
         item = NewRegexItem(C_END);
         regexArr->Append(regexArr, item);
         break;
      default:
         item = NewCharactor(c);
         regexArr->Append(regexArr, item);
         break;
      }
      current = item;
   }
   return regexArr;
}

int regex_line_match_backtrace(const char *str, Array *regexArr, int strIndex, int regexIndex)
{
   if (regexIndex >= regexArr->length)
      return strIndex;

   RegexItem *item = (RegexItem *)regexArr->Get(regexArr, regexIndex);

   if (item->category == C_BEGINNING)
   {
      return strIndex == 0 ? regex_line_match_backtrace(str, regexArr, 0, regexIndex + 1) : -1;
   }

   if (item->category == C_END)
   {
      return strIndex == strlen(str) ? strIndex : -1;
   }

   if (item->category == C_GROUP_CAPTURING)
      ERROR_LOG("this regex method cannot work with group capturing.");

   DEBUG_LOG("working on regex %d, position %d", regexIndex, strIndex)

   int min = item->repeatMin;
   int max = item->repeatMax;

   int left = strIndex, right = strIndex;
   int len = strlen(str);

   while (right < len && right - left < max && TryMatch(item, str[right])) // try matching chacters until reaches max repeat times.
      right++;
   // DEBUG_LOG("left: %d, right: %d\n", left, right);

   if (right - left < min)
   { // matching characters are less than minimum request.
      return -1;
   }

   while (right >= left + min)
   { // to the left most possible index.
      int r = regex_line_match_backtrace(str, regexArr, right, regexIndex + 1);
      if (r >= 0)
         return r;
      right--;
   }
   return -1;
}

/**
 * Deprecated, use regex_match_line instead.
 */
int regex_line_match(const char *line, Array *regexArr, int left)
{
   return regex_line_match_backtrace(line, regexArr, left, 0);
}

void read_regex(const char *filename, char *regex)
{
   FILE *fp;
   fp = fopen(filename, "r");
   if (fp == NULL)
      ERROR_LOG("failed to open regex file %s", filename);
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
      ERROR_LOG("failed to open file %s", filename);

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
      ERROR_LOG("Invalid arguments");
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
      int right = regex_line_match_backtrace(line, regexArr, left, 0);
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

   test("abcde", "a[^e]+e");
}
#endif