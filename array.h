#ifndef ARRAY
#define ARRAY

#define DELARR(arr, callback) DeleteArrayAndItsItems(arr, (void (*)(void *))callback)

typedef struct _Array Array;

struct _Array // define the struct of array
{
    int capacity;
    int length;
    void **items; // a dynamic array that stores the pointers of items

    int (*Append)(Array *arr, void *item); // define built-in functions
    void (*Reset)(Array *arr, void (*callback)(void *));
    void *(*Pop)(Array *arr);
    void *(*Get)(Array *arr, int index);
    void *(*Last)(Array *arr);
};

/**
 * Initialize a new array
 * returns pointer of the new array
 */
Array *NewArray();

/**
 * Free all memories recursively.
 * Array *arr: the array pointer that needs to be freed
 */
void DeleteArray(Array *arr);

/**
 * Free all memories recursively, including its items.
 * Array *arr: the array pointer that needs to be freed
 * void (*)(void*): the callback function to free item.
 */
void DeleteArrayAndItsItems(Array *arr, void (*)(void *));

#endif