#include "hw3.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int get_character_class_mode(char c);
/**
 * Init a new RegexItem
 * int category: category of the regex pattern
 * returns: pointer of the RegexItem
 */
RegexItem *NewRegexItem(int category)
{
   RegexItem *item = calloc(1, sizeof(RegexItem));
   item->category = category;
   item->repeatMax = 1;
   item->repeatMin = 1;
   return item;
}
/**
 * Init a new RegexItem
 * char c: character class
 * returns: pointer of the RegexItem
 */
RegexItem *NewCharacterClass(char c)
{
   DEBUG_LOG("new character class \\%c", c);
   RegexItem *item = NewRegexItem(C_CLASS);
   item->u.mode = get_character_class_mode(c);
   return item;
}
/**
 * Init a new RegexItem
 * char c: character class
 * returns: pointer of the RegexItem
 */
RegexItem *NewCharacter(char c)
{
   DEBUG_LOG("new character %c", c);
   RegexItem *item = NewRegexItem(C_SINGLE);
   item->u.c = c;
   return item;
}
/**
 * Init a new character group
 * returns: pointer of the RegexItem
 */
RegexItem *NewCharacterGroup()
{
   DEBUG_LOG("new character group");
   RegexItem *item = NewRegexItem(C_GROUP_INCLUSIVE);
   item->u.items = NewArray();
   return item;
}
/**
 * get the corresponding regex pattern type
 * char c: character to classify
 * returns: class type
 */
int get_character_class_mode(char c)
{
   switch (c)
   {
   case '.':                            // any char
      return CLASS_ANY;
   case 'd':                            // digit
      return CLASS_DIGIT;
   case 'D':                            // non-digit
      return CLASS_NDIGIT;
   case 'w':                            // letter
      return CLASS_LETTER;
   case 'W':                            // non-letter
      return CLASS_NLETTER;
   case 's':                            // whitespace
      return CLASS_WHITESPACE;
   case 'S':                            // non-whitespace
      return CLASS_NWHITESPACE;
   default:
      ERROR_LOG("Invalid special character: \\%c", c)
   }
   return 0;
}
/**
 * Parse the regex string
 * const char *regex: regex string
 * int *index: index
 * int inGroup: if it's inside ()
 * returns: a RegexItem array
 */
Array *parse_regex_array(const char *regex, int *index, int inGroup)
{
   Array *regexArr = NewArray();    // init the array
   int len = strlen(regex);

   if (*index < len)
   {
      switch (regex[*index])        // ignore the repetition
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
   for (; *index < len; (*index)++) // iterate thru the regex string
   {
      char c = regex[*index];

      switch (c)                    // find the corresponding pattern
      {
      case '.':
         item = NewCharacterClass(c); // a single . is a special character class.
         regexArr->Append(regexArr, item);
         break;
      case '\\':
         item = parse_escape_character(regex, index);
         regexArr->Append(regexArr, item);
         break;
      case '(':                     // a group begin
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
      case '^': // beginning
         if (*index != 0)
            ERROR_LOG("^ must present at the beginning of a valid regex string.")
         item = NewRegexItem(C_BEGINNING);
         DEBUG_LOG("beginning")
         regexArr->Append(regexArr, item);
         break;
      case '$': // end
         if (*index != len - 1)
            ERROR_LOG("$ must present at the end of a valid regex string.")
         DEBUG_LOG("ending")
         item = NewRegexItem(C_END);
         regexArr->Append(regexArr, item);
         break;
      default:
         item = NewCharacter(c);
         regexArr->Append(regexArr, item);
         break;
      }
   }
   return regexArr;
}
/**
 * Parse the escape character
 * const char *regex: regex string
 * int *index: index
 * returns: a RegexItem pointer
 */
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
      return NewCharacter(c);
   default:
      return NewCharacterClass(regex[*index]);
   }
}

/**
 * Parse the character group []
 * const char *regex: regex string
 * int *index: index
 * returns: a RegexItem pointer
 */
RegexItem *parse_character_group(const char *regex, int *index)
{
   if (regex[*index] != '[' || (*index + 1) >= strlen(regex))
      ERROR_LOG("character group should start with '[' and have at least length of 2")
   *index += 1; // move 1 step forward to skip '\\'

   RegexItem *group = NewCharacterGroup();  // init a new character group
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
         temp = NewCharacter(c);
         items->Append(items, temp);
         break;
      }
   }
   ERROR_LOG("character group should end with ']'")
   return NULL;
}

/**
 * Parse the capturing group ()
 * const char *regex: regex string
 * int *index: index
 * returns: a RegexItem pointer
 */
RegexItem *parse_capturing_group(const char *regex, int *index)
{
   if (regex[*index] != '(')
      ERROR_LOG("an capturing group must start with '('");
   *index += 1; // move 1 step forward to skip '('

   DEBUG_LOG("start a capturing group")
   RegexItem *group = NewRegexItem(C_GROUP_CAPTURING);      // init the group and continue parsing
   group->u.items = parse_regex_array(regex, index, 1);

   if (regex[*index] != ')')
      ERROR_LOG("an capturing group must end with ')'");
   DEBUG_LOG("end a capturing group, with %d items", group->u.items->length)
   return group;
}

/**
 * Parse the quantifiers *+? {n}
 * const char *regex: regex string
 * int *index: index
 * returns: a RegexItem pointer
 */
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
      if (c == ',')                 // if there's a ',' inside {}
      {
         if (state == 1)            // if already has a ','
            ERROR_LOG("quantifier format error, ',' should not appear twice or more.");
         current->repeatMin = num;  // set repetition times
         num = 0;
         state = 1;
      }
      else if (c == '}')            // end of the quantifier
      {
         if (state == 0)            // set the repetition times
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