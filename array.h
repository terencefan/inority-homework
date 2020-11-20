#ifndef ARRAY
#define ARRAY

typedef struct _Array Array;

struct _Array
{
    int capacity;
    int length;
    void **items;

    void (*Swap)(Array *arr, int i, int j);
    int (*Append)(Array *arr, void *item);
    Array *(*Concat)(Array *arr, Array *other);
    void *(*Pop)(Array *arr);
    void *(*Get)(Array *arr, int index);
};

Array *NewArray();

void DeleteArray(Array *arr);
#endif