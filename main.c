#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "hw1.h"
#include "map.h"
#include "heap.h"

#define STATE_NORMAL 0
#define STATE_SCRIPT 1
#define STATE_TAG 2

#define TYPE_MONOGRAM 0
#define TYPE_BIGRAM 1
#define TYPE_TRIGRAM 2

//====================These are helper functions=======================
int handle_line_normal(const char *line,
                       int pos,
                       Map *bigram,
                       Map *trigram,
                       Map *monogram,
                       Array* script_open,
                       Array* tag,
                       Array* tag_open,
                       Array* valid_word);
int handle_line_script(const char *line, int pos, Array* script_close);
int handle_line_tag(const char *line, int pos, Array* tag_close);
void insert_word(const char *string, Map *counter_map, int type);
void print_words(Heap *word_50);
void print_bigrams(Heap *bigram_20);
void print_trigrams(Heap *trigram_12);
//=====================function specs are at below=====================

// define some global varibles
int status = 0;
int doc_count = 0;
int word_counter = 0;
int unique_word_counter = 0;
int bigram_counter = 0;
int unique_bigram_counter = 0;
int trigram_counter = 0;
int unique_trigram_counter = 0;
char word_cache_1[50];
char word_cache_2[50];
char tmp_bigram[100];
char tmp_trigram[150];
char tmp_string[50];

/**
 * Check if a string is stop words from AP89 list, string length = 2
 * return 1: yes, 0: no
 * */
int search_stop_word_2(const char *string)
{
    if ((strcmp(string, "of") == 0) || (strcmp(string, "to") == 0) || (strcmp(string, "in") == 0))
        return 1;
    if ((strcmp(string, "on") == 0) || (strcmp(string, "he") == 0) || (strcmp(string, "is") == 0))
        return 1;
    if ((strcmp(string, "at") == 0) || (strcmp(string, "by") == 0) || (strcmp(string, "it") == 0))
        return 1;
    if ((strcmp(string, "as") == 0) || (strcmp(string, "be") == 0) || (strcmp(string, "an") == 0))
        return 1;
    if ((strcmp(string, "or") == 0) || (strcmp(string, "we") == 0) || (strcmp(string, "us") == 0))
        return 1;
    if ((strcmp(string, "up") == 0))
        return 1;
    return 0;
}
/**
 * Check if a string is stop words from AP89 list, string length = 3
 * return 1: yes, 0: no
 * */
int search_stop_word_3(const char *string)
{
    if ((strcmp(string, "the") == 0) || (strcmp(string, "and") == 0) || (strcmp(string, "for") == 0))
        return 1;
    if ((strcmp(string, "his") == 0) || (strcmp(string, "but") == 0) || (strcmp(string, "has") == 0))
        return 1;
    if ((strcmp(string, "are") == 0) || (strcmp(string, "who") == 0) || (strcmp(string, "its") == 0))
        return 1;
    if ((strcmp(string, "had") == 0) || (strcmp(string, "was") == 0) || (strcmp(string, "new") == 0))
        return 1;
    if ((strcmp(string, "one") == 0) || (strcmp(string, "not") == 0))
        return 1;
    return 0;
}
/**
 * Check if a string is stop words from AP89 list, string length = 4
 * return 1: yes, 0: no
 * */
int search_stop_word_4(const char *string)
{
    if ((strcmp(string, "said") == 0) || (strcmp(string, "that") == 0) || (strcmp(string, "with") == 0))
        return 1;
    if ((strcmp(string, "from") == 0) || (strcmp(string, "were") == 0) || (strcmp(string, "have") == 0))
        return 1;
    if ((strcmp(string, "they") == 0) || (strcmp(string, "will") == 0) || (strcmp(string, "been") == 0))
        return 1;
    if ((strcmp(string, "this") == 0) || (strcmp(string, "more") == 0))
        return 1;
    return 0;
}
/**
 * Check if a string is stop words from AP89 list, string length = 5
 * return 1: yes, 0: no
 * */
int search_stop_word_5(const char *string)
{
    if ((strcmp(string, "would") == 0) || (strcmp(string, "about") == 0) || (strcmp(string, "their") == 0))
        return 1;
    if ((strcmp(string, "which") == 0) || (strcmp(string, "after") == 0))
        return 1;
    return 0;
}
/**
 * Check if a string is html character
 * const char* line: the whole line
 * int pos: the position where the string starts
 * return: the length of the html position that matched.
 *         -1 if it isn't
 * */
int match_html_chars(const char *line, int pos)
{
    if (pos + 6 < strlen(line))             // if line[pos+6] not out of bound
    {
        char string[7];                     // copy the string line[pos:pos+6]
        memset(string, '\0', 7);
        strncpy(string, line + pos, 6);     // compare it to html characters with length 6
        if (strcmp(string, "&nbsp;") == 0)
        {
            return 5;                       // return the html character length if matched
        }
        if (strcmp(string, "&quot;") == 0)
        {
            return 5;
        }
    }
    if (pos + 5 < strlen(line))             // if line[pos+4] not out of bound
    {
        char string[6];                     // copy the string line[pos:pos+5]
        memset(string, '\0', 6);
        strncpy(string, line + pos, 5);     // compare it to html characters with length 5
        if (strcmp(string, "&amp;") == 0)
        {
            return 4;                       // return the html character length if matched
        }
    }
    if (pos + 4 < strlen(line))             // if line[pos+3] not out of bound
    {
        char string[5];                     // copy the string line[pos:pos+4]
        memset(string, '\0', 5);
        strncpy(string, line + pos, 4);
        if (strcmp(string, "&lt;") == 0)    // compare it to html characters with length 5
        {
            return 3;
        }
        if (strcmp(string, "&gt;") == 0)    // return the html character length if matched
        {
            return 3;
        }
    }
    return -1;                              // return -1 if not matched
}

/**
 * Read the document and process it
 * const char *file: filename
 * Map *bigram, Map *trigram, Map *monogram: maps for word frequencies: <string, frequency>
 * */
void handle_doc(const char *file,
                Map *bigram,
                Map *trigram,
                Map *monogram)
{
    // open the file
    char *line = NULL;
    size_t len = 0;
    size_t read;
    FILE *fp = fopen(file, "r");
    if (fp == NULL)
        ERROR_LOG("Cannot open file %s", file);
    doc_count++;

    Array *script_open = build_regex_array("<script");      // init all necessary regex arrays
    Array *tag = build_regex_array("<[^<>]+>");
    Array *valid_word = build_regex_array("\\w+'?\\w+");
    Array *tag_open = build_regex_array("<[^<>]+\\s");
    Array *tag_close = build_regex_array(">");
    Array *script_close = build_regex_array("</script>");

    while ((read = getline(&line, &len, fp)) != -1)                 // start interating thru the doc
    {
        int pos = 0; // current position in line
        while (pos < strlen(line))
        {
            if (status == STATE_NORMAL)         // if state is normal
            {
                pos = handle_line_normal(line, pos, bigram, trigram, monogram, script_open, tag, tag_open, valid_word);
                continue;
            }
            else if (status == STATE_SCRIPT)    // if looking for close script tags
            {
                pos = handle_line_script(line, pos, script_close);
                continue;
            } else if (status == STATE_TAG)     // if looking for other close tags
            {
                pos = handle_line_tag(line, pos, tag_close);
                continue;
            }
        }
    }
    // free everything
    DeleteArray(script_close);
    DeleteArray(script_open);
    DeleteArray(tag_close);
    DeleteArray(tag_open);
    DeleteArray(valid_word);
    DeleteArray(tag);
    fclose(fp);
}

/**
 * Looking for closing tag "/?>" only
 * const char *line: the line we are searching at
 * int pos: position in the line
 * Array* tag_close: regex item array of "/?>"
 * returns: the end position of the closing tag
 * */
int handle_line_tag(const char *line, int pos, Array* tag_close)
{
    int line_iter;
    for (line_iter = pos; line_iter < strlen(line); line_iter++)    // start iterating from the given position
    {
        // try match every character
        int right = regex_line_match(line, tag_close, line_iter);
        if (right != -1)
        {
            status = STATE_NORMAL; // change the state and find the closing tag
            return right - 1;      // return the position
        }
    }
    return line_iter;
}

/**
 * Looking for closing script tag "</script>" only
 * const char *line: the line we are searching at
 * int pos: position in the line
 * Array* script_close: regex item array of "</script>"
 * returns: the end position of the closing tag
 * */
int handle_line_script(const char *line, int pos, Array* script_close)
{
    int line_iter;
    for (line_iter = pos; line_iter < strlen(line); line_iter++)    // iterate from the given position
    {
        int right;
        if (line[line_iter] == '<') // if there's a '<', try match
        {
            right = regex_line_match(line, script_close, line_iter);
            if (right != -1)
            {
                status = STATE_NORMAL; // change the state and find the closing tag

                return right - 1;       // return the position
            }
        }
    }
    return line_iter;
}

/**
 * Ignoring all special characters in line and find valid words
 * const char *line: the line we are searching at
 * int pos: position in the line
 * Map *bigram,Map *trigram,Map *monogram: maps to store word frequencies
 * Array* script_open: regex item array of "<script"
 * Array* tag: regex item array "<[^<>]>"
 * Array* tag_open: regex item array "<[^<>]"
 * Array* valid_word: regex item array "\w'?\w"
 * returns: the position of where the line is currently handled
 * */
int handle_line_normal(const char *line,
                       int pos,
                       Map *bigram,
                       Map *trigram,
                       Map *monogram,
                       Array* script_open,
                       Array* tag,
                       Array* tag_open,
                       Array* valid_word)
{
    int line_iter;              // iterating thru each character
    for (line_iter = pos; line_iter < strlen(line); line_iter++)
    {
        int right;
        if (line[line_iter] == '<')
        { // check for '<'
            // try to find a open script tag
            // then the script tag must span multiple lines
            right = regex_line_match(line, script_open, line_iter);
            if (right != -1)
            {                             // if found the beginning of the script tag
                status = STATE_SCRIPT;    // change the state and find the closing tag
                return right - 1; // return the position of the script tage
            }
            // if script tag not found, try to find other tags
            right = regex_line_match(line, tag, line_iter);
            if (right != -1)
            {
                // if other tags found, ignore the whole tag, jump to the end of the tag
                line_iter = right - 1;
            }

            right = regex_line_match(line,tag_open,line_iter); // try to find if there's an open tag
            if (right != -1)
            {
                // if other tags found, ignore the whole tag, jump to the end of the tag
                status = STATE_TAG;
                return right - 1;
            }
        }
        else if (line[line_iter] == '&')            // check for html characters
        {
            int result = match_html_chars(line, line_iter);
            if (result != -1)                       // if found
            {
                line_iter += result;                // jump to the position
                continue;
            }
        }
        else if (!isalpha(line[line_iter]))
        { // if this is other special characters, simply ignore
            continue;
        }
        else
        { // if this current character is a letter
            int flag = 0;
            if ((line_iter - 1 < 0) || !isalpha(line[line_iter - 1]))
                flag = 1;
            if (flag)
            { // check if this letter is the beginning of a word
                // search for the valid word
                right = regex_line_match(line, valid_word, line_iter);
                if (right == -1)
                { // no valid word found
                    continue;
                }
                else
                { // found valid word
                    memset(tmp_string, '\0', 50);                           // reset the cache
                    strncpy(tmp_string, line + line_iter, right - line_iter);   // copy result into the cache
                    tmp_string[right - line_iter] = '\0';
                    int str_iter;
                    for (str_iter = 0; str_iter < strlen(tmp_string); str_iter++)
                    {                                                         // iterate thru the string
                        tmp_string[str_iter] = tolower(tmp_string[str_iter]); // set char to lower case
                    }
                    insert_word(tmp_string, monogram, TYPE_MONOGRAM); // insert into frequency map
                    // check if it is a stop word in AP89
                    int result = 0;
                    if (strlen(tmp_string) == 2)                    // search by the length of the result
                        result = search_stop_word_2(tmp_string);
                    else if (strlen(tmp_string) == 3)
                        result = search_stop_word_3(tmp_string);
                    else if (strlen(tmp_string) == 4)
                        result = search_stop_word_4(tmp_string);
                    else if (strlen(tmp_string) == 5)
                        result = search_stop_word_5(tmp_string);
                    else if (strlen(tmp_string) == 6)
                        result = (strcmp("people", tmp_string) == 0);
                    else if (strlen(tmp_string) == 7)
                        result = (strcmp("percent", tmp_string) == 0);
                    if (result == 0)               // if it isn't a stop word
                    {
                        // read from word cache to build bigram and trigrams
                        int word_1_len = strlen(word_cache_1);
                        int word_2_len = strlen(word_cache_2);
                        if (word_2_len > 0)
                        { // build bigram
                            memset(tmp_bigram, '\0', 100);
                            strcpy(tmp_bigram, word_cache_2);
                            strcpy(tmp_bigram + word_2_len, " ");
                            strcpy(tmp_bigram + word_2_len + 1, tmp_string);
                            insert_word(tmp_bigram, bigram, TYPE_BIGRAM);       // insert into frequency map
                        }
                        if (word_1_len > 0 && word_2_len > 0)
                        { // build trigram
                            memset(tmp_trigram, '\0', 150);
                            strcpy(tmp_trigram, word_cache_1);
                            strcpy(tmp_trigram + word_1_len, " ");
                            strcpy(tmp_trigram + word_1_len + 1, tmp_bigram);
                            insert_word(tmp_trigram, trigram, TYPE_TRIGRAM);    // insert into frequency map
                        }
                        memset(word_cache_1, '\0', 50); // reset the word cache
                        strcpy(word_cache_1, word_cache_2);
                        memset(word_cache_2, '\0', 50);
                        strcpy(word_cache_2, tmp_string);
                    }
                    else
                    {                                   // if it is a stop word
                        memset(word_cache_1, '\0', 50); // clear the cache
                        memset(word_cache_2, '\0', 50);
                    }
                    line_iter = right - 1;              // jump to the end of the valid word, continue processing
                }
            }
        }
    }
    return line_iter;
}

/**
 * Iterate a string and its value into the frequency map.
 * const char *string: key of the map
 * Map *counter_map: the frequency map
 * int type: if this is a monogram, bigram or trigram
 * */
void insert_word(const char *string, Map *counter_map, int type)
{
    int *cnt = counter_map->Get(counter_map, string);   // see if the key already exists
    switch (type)                   // increment the word count by type
    {
    case TYPE_MONOGRAM:
        word_counter++;
        break;
    case TYPE_BIGRAM:
        bigram_counter++;
        break;
    case TYPE_TRIGRAM:
        trigram_counter++;
        break;
    default:
        break;
    }
    if (cnt == NULL)            // if the string not exist in map, this is a unique word
    {
        cnt = (int *)calloc(1, sizeof(int));
        *cnt = 1;
        counter_map->Add(counter_map, string, cnt); // add it into the map with frequency 1
        switch (type)                               // increment the unique word count
        {
        case TYPE_MONOGRAM:
            unique_word_counter++;
            break;
        case TYPE_BIGRAM:
            unique_bigram_counter++;
            break;
        case TYPE_TRIGRAM:
            unique_trigram_counter++;
            break;
        default:
            break;
        }
    }
    else
    {
        (*cnt)++;                       // if it exists in the map, it is not unique, simply increment the frequency
    }
}

/**
 * Iterate thru the map, push elements in to the min heap.
 * Heap* minheap: the min heap
 * Map* map: the map for words
 * */
void parse(Heap *minheap, Map *map)
{
    MapIterator *mapIterator = NewMapIterator(map); // create a map iterator
    const char *string;                             // pointer to store key and values
    int *value;
    while (mapIterator->current)                    // start iteration
    {
        string = mapIterator->current->key;         // read key and value
        value = mapIterator->current->val;
        if (minheap->size == minheap->capacity)     // if the heap is full
        {
            int freq;
            minheap->Top(minheap, &freq);
            if (freq < *value)                      // compare the frequency on top of the heap with the current one
            {                                       // if the current one is larger
                void* top_pointer = minheap->Pop(minheap, &freq);       // pop the heap and push the current one
                free(top_pointer);
                char *heap_item = calloc(1, strlen(string));
                strcpy(heap_item, string);
                minheap->Push(minheap, *value, heap_item);
            }
        }
        else
        {                                           // if the heap is not full
            char *heap_item = calloc(1, strlen(string));    // simply push the current value into the heap
            strcpy(heap_item, string);
            minheap->Push(minheap, *value, heap_item);
        }
        mapIterator->Next(mapIterator);
    }
    DeleteMapIterator(mapIterator);                 // free the map iterator
}

/**
 * Print out counter numbers
 * */
void print_results()
{
    printf("Total number of documents: %d\n", doc_count);
    printf("Total number of words: %d\n", word_counter);
    printf("Total number of unique words: %d\n", unique_word_counter);
    printf("Total number of interesting bigrams: %d\n", bigram_counter);
    printf("Total number of unique interesting bigrams: %d\n", unique_bigram_counter);
    printf("Total number of interesting trigrams: %d\n", trigram_counter);
    printf("Total number of unique interesting trigrams: %d\n", unique_trigram_counter);
    printf("\n");
}

/**
 * Print out top 50 words
 * Heap* word_50: min heap for monograms(single words)
 * */
void print_words(Heap *word_50)
{
    char *stack[50];                        // cache to store string and frequencies
    int freq[50];
    int counter1 = word_50->size - 1;       // two counters to help iterate through the heap
    int counter2 = word_50->size - 1;
    char *string;
    while (counter1 >= 0)                   // pop everything out and put them in the cache
    {
        string = word_50->Pop(word_50, &freq[counter1]);
        stack[counter1] = string;
        counter1--;
    }
    printf("Top 50 words:\n");
    counter1 = 0;
    while (counter1 <= counter2)            // print everything from the cache
    {
        printf("%d %s\n", freq[counter1], stack[counter1]);
        counter1++;
    }
    printf("\n");
}

/**
 * Print out top 20 bigrams
 * Heap* bigram_20: min heap for bigrams
 * */
void print_bigrams(Heap *bigram_20)
{
    char *stack[20];                        // cache to store string and frequencies
    int freq[20];
    int counter1 = bigram_20->size - 1;     // two counters to help iterate through the heap
    int counter2 = bigram_20->size - 1;
    char *string;
    while (counter1 >= 0)
    {                                       // pop everything out and put them in the cache
        string = bigram_20->Pop(bigram_20, &freq[counter1]);
        stack[counter1] = string;
        counter1--;
    }
    printf("Top 20 interesting bigrams:\n");
    counter1 = 0;
    while (counter1 <= counter2)            // print everything from the cache
    {
        printf("%d %s\n", freq[counter1], stack[counter1]);
        counter1++;
    }
    printf("\n");
}

/**
 * Print out top 12 bigrams
 * Heap* trigram_12: min heap for trigrams
 * */
void print_trigrams(Heap *trigram_12)
{
    char *stack[12];                        // cache to store string and frequencies
    int freq[12];
    int counter1 = trigram_12->size - 1;    // two counters to help iterate through the heap
    int counter2 = trigram_12->size - 1;
    char *string;
    while (counter1 >= 0)
    {                                       // pop everything out and put them in the cache
        string = trigram_12->Pop(trigram_12, &freq[counter1]);
        stack[counter1] = string;
        counter1--;
    }
    printf("Top 12 interesting trigrams:\n");
    counter1 = 0;
    while (counter1 <= counter2)            // print everything from the cache
    {
        printf("%d %s\n", freq[counter1], stack[counter1]);
        counter1++;
    }
    printf("\n");
}

/**
 * Start running
 * int argc: cmd argument count
 * char * argv[]: cmd arguments
 * */
void run(int argc, char *argv[])
{
    // init data structrues
    Map *bigram = NewMap();
    Map *trigram = NewMap();
    Map *monogram = NewMap();
    Heap *word_50 = NewMinHeap(50);
    Heap *bigram_20 = NewMinHeap(20);
    Heap *trigram_12 = NewMinHeap(12);
    int iter;
    // iterate thru documents and read them
    for (iter = 1; iter < argc; iter++)
    {
        handle_doc(argv[iter], bigram, trigram, monogram);  // handle each document
    }
    // parse results
    parse(word_50, monogram);
    parse(bigram_20, bigram);
    parse(trigram_12, trigram);
    // print results
    print_results();
    print_words(word_50);
    print_bigrams(bigram_20);
    print_trigrams(trigram_12);
    // free everything
    DeleteMap(bigram, 1);
    DeleteMap(trigram, 1);
    DeleteMap(monogram, 1);
    DeleteHeap(word_50);
    DeleteHeap(bigram_20);
    DeleteHeap(trigram_12);
}

#ifndef USE_SUBMITTY_MAIN
int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc < 2)
    { // check arguments
        fprintf(stderr, "ERROR: Invalid arguments\n");
        fprintf(stderr, "USAGE: a.out <input-file1> [ <input-file2> ... ]\n");
    }
    // start running
    run(argc, argv);
    return 0;
}
#endif