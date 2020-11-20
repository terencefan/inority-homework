#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw1.h"

#define CLASS_ANY 1
#define CLASS_DIGIT 2
#define CLASS_NDIGIT 3
#define CLASS_LETTER 4
#define CLASS_NLETTER 5
#define CLASS_WHITESPACE 6

#define C_SINGLE 1 // a single character
#define C_CLASS 2  // character class, like '.', '\\', '\w', '\d'
#define C_GROUP 3  // character group, like '[abc]'

#define T_1 1
#define T_01 2 // ?
#define T_1N 3 // +
#define T_0N 4 // *

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

typedef struct
{
   int type;
   Array *items;
} CharacterGroup;

typedef struct
{
   int wildcardType;
   int category;

   union
   {
      char c;                // for a single character
      int mode;              // for character class
      CharacterGroup *group; // for character group
   } u;

} RegexItem;

int parse_mode(char c)
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
   default:
      ERROR_LOG("Invalid special character: \\%c", c);
   }
   return 0;
}

RegexItem *NewCharacterClass(char c)
{
   DEBUG_LOG("new character class \\%c", c);
   RegexItem *item = malloc(sizeof(RegexItem));
   item->u.mode = parse_mode(c);
   item->category = C_CLASS;
   item->wildcardType = T_1;
   return item;
}

RegexItem *NewCharactor(char c)
{
   DEBUG_LOG("new character %c", c);
   RegexItem *item = malloc(sizeof(RegexItem));
   item->category = C_SINGLE;
   item->wildcardType = T_1;
   item->u.c = c;
   return item;
}

RegexItem *NewCharacterGroup()
{
   DEBUG_LOG("new character group");
   RegexItem *item = malloc(sizeof(RegexItem));
   item->category = C_GROUP;
   item->wildcardType = T_1;
   item->u.group = malloc(sizeof(CharacterGroup));
   item->u.group->type = INCLUSIVE;
   item->u.group->items = NewArray();
   return item;
}

void DeleteRegexItem(RegexItem *item)
{
   switch (item->category)
   {
   case C_GROUP:
      DeleteArray(item->u.group->items);
      free(item->u.group);
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
      }
   }
   else if (item->category == C_GROUP)
   {
      CharacterGroup *group = item->u.group;
      Array *items = group->items;
      for (int itemIndex = 0; itemIndex < items->length; itemIndex++)
      {
         RegexItem *subItem = items->Get(items, itemIndex);
         int matched = TryMatch(subItem, c);
         if (matched)
            return group->type == INCLUSIVE;
      }
      return group->type == EXCLUSIVE;
   }
   else
      ERROR_LOG("unknown regex item category: %d", item->category);
   return 0;
}

RegexItem *parse_character_class(const char *regex, int *index)
{
   if (regex[*index] != '\\' || (*index + 1) >= strlen(regex))
      ERROR_LOG("character class should start with '\\'");
   *index += 1; // move 1 step forward to skip '\\'

   char c = regex[*index];
   if (c == '.' || c == '\\')
   {
      // \. and \\ are not character class.
      return NewCharactor(c);
   }
   return NewCharacterClass(regex[*index]);
}

RegexItem *parse_character_group(const char *regex, int *index)
{
   if (regex[*index] != '[')
      ERROR_LOG("character group should start with '['");

   RegexItem *group = NewCharacterGroup();
   RegexItem *temp;
   Array *items = group->u.group->items;

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
         group->u.group->type = EXCLUSIVE;
         break;
      case '.': // A single '.' is also a character class.
         temp = NewCharacterClass(c);
         items->Append(items, temp);
         break;
      case '\\':
         temp = parse_character_class(regex, index);
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
   Array *regexArr = NewArray();
   RegexItem *current = NULL;

   if (strlen(regex) > 0)
   {
      if (regex[0] == '+' || regex[0] == '?' || regex[0] == '*')
      {
         ERROR_LOG("regex string couldn't start with wildcard characters");
      }
   }

   for (int i = 0; i < strlen(regex); i++)
   {
      char c = regex[i];
      RegexItem *item = current;

      switch (c)
      {
      case '?':
         current->wildcardType = T_01;
         DEBUG_LOG("set wildcardType to ?");
         break;
      case '+':
         current->wildcardType = T_1N;
         DEBUG_LOG("set wildcardType to +");
         break;
      case '*':
         current->wildcardType = T_0N;
         DEBUG_LOG("set wildcardType to *");
         break;
      case '.':
         item = NewCharacterClass(c);
         regexArr->Append(regexArr, item);
         break;
      case '\\':
         item = parse_character_class(regex, &i);
         regexArr->Append(regexArr, item);
         break;
      case '[':
         item = parse_character_group(regex, &i);
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
   if (regexIndex == regexArr->length)
      return strIndex;

   DEBUG_LOG("str index: %d, regex index: %d", strIndex, regexIndex);

   RegexItem *item = (RegexItem *)regexArr->Get(regexArr, regexIndex);
   if (item == NULL)
      ERROR_LOG("array get out of bounds");
   // DEBUG_LOG("regex item category: %d,: regex item wildcard type: %d", item->category, item->wildcardType);

   int left = strIndex, right = strIndex;
   if (TryMatch(item, str[left]))
      right++;
   else if (item->wildcardType == T_1 || item->wildcardType == T_1N)
      return -1;

   switch (item->wildcardType)
   {
   case T_1N:
      left++; // skip first character
   case T_0N:
      // find the right most matched character.
      while (right < strlen(str) && TryMatch(item, str[right]))
         right++;
      break;
   case T_1:
      left++; // skip first character
   case T_01:
      break;
   default:
      ERROR_LOG("unknown repeat type: %d", item->wildcardType);
   }

   DEBUG_LOG("left: %d,: right %d", left, right);

   for (; right >= left; right--)
   {
      int r = regex_line_match_backtrace(str, regexArr, right, regexIndex + 1);
      if (r < 0)
         continue;
      return r;
   }
   return -1;
}

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
         int right = regex_line_match_backtrace(line, regexArr, left, 0);
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

   // if (argc < 3) {
   //    ERROR_LOG("Invalid arguments");
   // }
   // char* regex_file = argv[1];
   // char* text_file = argv[2];

   char *regex_file = "regex.txt";
   char *text_file = "hw1-input01.txt";
   read_regex(regex_file, regex);
   int count = regex_match(text_file, regex, &matches, 1);
   for (int i = 0; i < count; i++)
   {
      printf("%s", matches[i]);
   }
   return 0;
}
#endif