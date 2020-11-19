typedef struct _Array Array;

struct _Array
{
    int capacity;
    int length;
    void **items;

    int (*Append)(Array *arr, void *item);
    void* (*Pop)(Array *arr);
    void *(*Get)(Array *arr, int index);
};

Array *NewArray();

void DeleteArray(Array *arr);