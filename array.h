#ifndef ARRAY
#define ARRAY

typedef struct _Array Array;

struct _Array // define the struct of array
{
    int capacity;
    int length;
    void **items; // a dynamic array that stores the pointers of items

    int (*Append)(Array *arr, void *item); // define built-in functions
    void *(*Pop)(Array *arr);
    void (*Swap)(Array *arr, int i, int j);
    Array *(*Concat)(Array *arr, Array *other);
    void *(*Get)(Array *arr, int index);
};

/**
 * Initialize a new array
 * returns pointer of the new array
 */
Array *NewArray();

/**
 * Free all memories recursively, including its items.
 * Array *arr: the array pointer that needs to be freed
 */
void DeleteArray(Array *arr);

/**
 * Append the item to the array
 * Array *arr: the array pointer
 * void *item: pointer of the item
 * returns: 0 by default
 */
int ArrayAppend(Array *arr, void *item);

/**
* Pop the last item from the array
* Array *arr: the array pointer
* returns: pointer of the last item
*/
void *ArrayPop(Array *arr);

/**
* Get an array item by index
* Array *arr: the array pointer
* int index: the index
* returns: pointer of the item from arr[index]
*/
void *ArrayGet(Array *arr, int index);

/**
* Free the array struct but keep the list of items
* Array *arr: the array pointer
* returns: pointer of the item list
*/
void **RemoveArray(Array *arr);

#endif