#include "hw1.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_character_class_mode(char c);

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
         regexArr->Append(regexArr, item);
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
      DEBUG_LOG("character group set to exclusive mode");
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

RegexItem *parse_capturing_group(const char *regex, int *index)
{
   if (regex[*index] != '(')
      ERROR_LOG("an capturing group must start with '('");
   *index += 1; // move 1 step forward to skip '('

   DEBUG_LOG("start a capturing group")
   RegexItem *group = NewRegexItem(C_GROUP_CAPTURING);
   group->u.items = parse_regex_array(regex, index, 1);

   if (regex[*index] != ')')
      ERROR_LOG("an capturing group must end with ')'");
   DEBUG_LOG("end a capturing group, with %d items", group->u.items->length)
   return group;
}

RegexItem *parse_quantifiers(const char *regex, int *index, RegexItem *current)
{
   char first = regex[*index];

   switch (first)
   {
   case '?':
      current->repeatMin = 0;
      current->repeatMax = 1;
      DEBUG_LOG("set wildcard to ?");
      return current;
   case '+':
      current->repeatMin = 1;
      current->repeatMax = INF;
      DEBUG_LOG("set wildcard to +");
      return current;
   case '*':
      current->repeatMin = 0;
      current->repeatMax = INF;
      DEBUG_LOG("set wildcard to *");
      return current;
   case '{':
      break; // simply skip for furthur parsing process.
   default:
      ERROR_LOG("quantifier should start with '{'");
   }

   int len = strlen(regex);
   int num = 0;
   int state = 0; // 0: unmet ',' 1: met ','
   *index += 1;

   while ((*index) < len)
   {
      char c = regex[*index];
      if (c == ',')
      {
         if (state == 1)
            ERROR_LOG("quantifier format error, ',' should not appear twice or more.");
         current->repeatMin = num;
         num = 0;
         state = 1;
      }
      else if (c == '}')
      {
         if (state == 0)
            current->repeatMin = current->repeatMax = num;
         else if (num > 0)
            current->repeatMax = num;
         else
            current->repeatMax = INF;
         DEBUG_LOG("set repetetion to {%d, %d}", current->repeatMin, current->repeatMax);
         return current;
      }
      else if (isdigit(c))
      {
         num = num * 10 + (int)(c - '0');
      }
      else
         ERROR_LOG("quatifier format error, '%c' is not allowed.", c);
      (*index)++;
   }
   ERROR_LOG("quantifier should end with '}'");
   return NULL;
}