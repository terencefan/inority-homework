#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw1.h"
#include "map.h"
#include "heap.h"

#define STATE_NORMAL 1
#define STATE_SCRIPT 2
#define STATE_TAG 3

typedef struct _Counter Counter;

int is_stopword(const char *word);

struct _Counter
{
    int documentCount;

    int monogramCount;
    int bigramCount;
    int trigramCount;

    char *w1;
    char *w2;

    Map *monogramMap;
    Map *bigramMap;
    Map *trigramMap;

    void (*feed)(Counter *this, const char *word);
    void (*stop)(Counter *this);
};

void incr(Map *map, const char *key)
{
    int *count = map->Get(map, key);
    if (count == NULL)
    {
        count = calloc(1, sizeof(int));
        map->Add(map, key, count);
    }
    (*count)++;
}

void incr_monogram(Counter *this, const char *word)
{
    this->monogramCount++;
    incr(this->monogramMap, word);
}

void incr_bigram(Counter *this, const char *w1, const char *w2)
{
    char word[256] = "";
    strcat(word, w1);
    strcat(word, " ");
    strcat(word, w2);
    this->bigramCount++;
    incr(this->bigramMap, word);
}

void incr_trigram(Counter *this, const char *w1, const char *w2, const char *w3)
{
    char word[256] = "";
    strcat(word, w1);
    strcat(word, " ");
    strcat(word, w2);
    strcat(word, " ");
    strcat(word, w3);
    this->trigramCount++;
    incr(this->trigramMap, word);
}

void feed(Counter *this, const char *word)
{
    incr_monogram(this, word);
    if (is_stopword(word))
    {
        this->stop(this);
        return;
    }

    if (this->w2 != NULL)
        incr_bigram(this, this->w2, word);
    if (this->w1 != NULL)
        incr_trigram(this, this->w1, this->w2, word);

    if (this->w1 != NULL)
        free(this->w1);
    this->w1 = this->w2;
    this->w2 = calloc(1, strlen(word) + 1);
    strcpy(this->w2, word);
}

void stop(Counter *this)
{
    if (this->w1 != NULL)
        free(this->w1);
    if (this->w2 != NULL)
        free(this->w2);
    this->w1 = this->w2 = NULL;
}

Counter *NewCounter()
{
    Counter *counter = calloc(1, sizeof(Counter));
    counter->monogramMap = NewMap();
    counter->bigramMap = NewMap();
    counter->trigramMap = NewMap();
    counter->w1 = NULL;
    counter->w2 = NULL;
    counter->feed = feed;
    counter->stop = stop;
    return counter;
}

void DeleteCounter(Counter *counter)
{
    DeleteMap(counter->monogramMap, DELETE_VAL);
    DeleteMap(counter->bigramMap, DELETE_VAL);
    DeleteMap(counter->trigramMap, DELETE_VAL);
    free(counter);
}

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

int is_stopword(const char *word)
{
    switch (strlen(word))
    {
    case 2:
        return search_stop_word_2(word);
    case 3:
        return search_stop_word_3(word);
    case 4:
        return search_stop_word_4(word);
    case 5:
        return search_stop_word_5(word);
    }
    return 0;
}

int is_html_chars(const char *line, int start, int end)
{
    switch (end - start)
    {
    case 6:
        return strncmp(line + start, "&nbsp;", 6) == 0 || strncmp(line + start, "&quot;", 6) == 0;
    case 5:
        return strncmp(line + start, "&amp;", 5) == 0;
    case 4:
        return strncmp(line + start, "&lt;", 4) == 0 || strncmp(line + start, "&gt;", 4) == 0;
    }
    return 0;
}

int CustomCompare(int w1, int w2, void *data1, void *data2)
{
    if (w1 == w2)
        return -strcmp(data1, data2);
    return (w1 - w2) < 0 ? -1 : (w1 - w2) > 0 ? 1 : 0;
}

void printTopK(Map *map, int k)
{
    Heap *heap = NewMinHeap(k);
    heap->Compare = CustomCompare;

    MapIterator *iter = NewMapIterator(map);
    int weight;
    for (; iter->current != NULL; iter->Next(iter))
    {
        MapEntity *current = iter->current;
        int count = *(int *)current->val;
        heap->Push(heap, count, (void *)current->key);
    }

    int weights[50];
    char *words[50];
    int index = 0;

    char *key;
    while (NULL != (key = heap->Pop(heap, &weight)))
    {
        weights[index] = weight;
        words[index] = key;
        index++;
    }

    for (index--; index >= 0; index--)
        printf("%d %s\n", weights[index], words[index]);
    DeleteHeap(heap);
}

Array *word, *openingScript, *closingScript, *htmlTag, *htmlChars;

void find_last_gt(const char *filename, int *x, int *y)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    size_t read;

    fp = fopen(filename, "r");
    if (fp == NULL)
        ERROR_LOG("failed to open file %s", filename);

    int lineno = -1;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        lineno++;
        for (int i = 0; line[i] != '\0'; i++)
            if (line[i] == '>')
            {
                *x = lineno;
                *y = i;
            }
    }
}

void parse_file(Counter *counter, const char *filename)
{
    counter->documentCount++;

    int lastGtX, lastGtY;
    find_last_gt(filename, &lastGtX, &lastGtY);

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    size_t read;

    fp = fopen(filename, "r");
    if (fp == NULL)
        ERROR_LOG("failed to open file %s", filename);

    int state = STATE_NORMAL, i, r;
    int lineno = -1;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        lineno++;
        for (i = 0; line[i] != '\0'; i++)
        {
            char c = line[i];
            switch (state)
            {
            case STATE_NORMAL:
                if (isblank(c))
                {
                    // just skip
                }
                else if (c == '<')
                {
                    // switch to script state.
                    r = regex_line_match(line, openingScript, i);
                    if (r > 0)
                    {
                        DEBUG_LOG("%sfound <script> at: (%d, %d)\n", line, i, r);
                        i = r - 1;
                        state = STATE_SCRIPT;
                        break;
                    }

                    if (lineno < lastGtX || (lineno == lastGtX && i < lastGtY))
                        state = STATE_TAG;
                }
                else if (c == '&')
                {
                    r = regex_line_match(line, htmlChars, i);
                    if (r > 0 && is_html_chars(line, i, r))
                    {
                        i = r - 1;
                        break;
                    }
                }
                else
                {
                    r = regex_line_match(line, word, i);
                    if (r < 0 || r - i < 2)
                        break;
                    DEBUG_LOG("%sfound word at: (%d, %d)\n", line, i, r);

                    char buf[256];
                    int offset = 0;
                    for (; i < r; i++, offset++)
                        buf[offset] = tolower(line[i]);
                    buf[offset] = '\0';
                    i--;

                    counter->feed(counter, buf);
                }
            case STATE_SCRIPT:
                if (c == '<')
                {
                    r = regex_line_match(line, closingScript, i);
                    if (r > 0)
                    {
                        DEBUG_LOG("%sfound </script> at: (%d, %d)\n", line, i, r);
                        i = r - 1;
                        state = STATE_NORMAL;
                        break;
                    }
                }
                break;
            case STATE_TAG:
                if (c == '>')
                    state = STATE_NORMAL;
                break;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    word = build_regex_array("\\w[\\w']+");
    openingScript = build_regex_array("<script.*>");
    closingScript = build_regex_array("</script.*>");
    htmlTag = build_regex_array("<.*[\\w\\d]+.*>");
    htmlChars = build_regex_array("&\\w+;");

    Counter *counter = NewCounter();
    if (argc < 2)
        ERROR_LOG("Invalid arguments");
    for (int index = 1; index < argc; index++)
        parse_file(counter, argv[index]);

    printf("Total number of documents: %d\n", counter->documentCount);
    printf("Total number of words: %d\n", counter->monogramCount);
    printf("Total number of unique words: %d\n", counter->monogramMap->Count(counter->monogramMap));
    printf("Total number of interesting bigrams: %d\n", counter->bigramCount);
    printf("Total number of unique interesting bigrams: %d\n", counter->bigramMap->Count(counter->bigramMap));
    printf("Total number of interesting trigrams: %d\n", counter->trigramCount);
    printf("Total number of unique interesting trigrams: %d\n", counter->trigramMap->Count(counter->trigramMap));

    printf("\nTop 50 words:\n");
    printTopK(counter->monogramMap, 50);
    printf("\nTop 20 interesting bigrams:\n");
    printTopK(counter->bigramMap, 20);
    printf("\nTop 12 interesting trigrams:\n");
    printTopK(counter->trigramMap, 12);

    DeleteCounter(counter);
}