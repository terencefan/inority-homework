#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

const int DEFAULT_ARR_CAPACITY = 32;

int ArrayAppend(Array *arr, void *item);
void ArrayReset(Array *arr, void (*callback)(void *));
void *ArrayPop(Array *arr);
void *ArrayGet(Array *arr, int index);
void *ArrayLast(Array *arr);

void clean(Array *arr, void (*callback)(void *))
{
   for (int index = 0; index < arr->length; index++)
      callback(arr->items[index]);
}

/**
 * Initialize a new array
 * returns pointer of the new array
 */
Array *NewArray()
{
   Array *arr = calloc(1, sizeof(Array)); // alloc memory
   arr->capacity = DEFAULT_ARR_CAPACITY;  // set property to default
   arr->length = 0;
   arr->items = calloc(DEFAULT_ARR_CAPACITY, sizeof(void *));

   arr->Append = ArrayAppend; // set built-in methods
   arr->Reset = ArrayReset;
   arr->Pop = ArrayPop;
   arr->Get = ArrayGet; 
   arr->Last = ArrayLast;
   return arr;
}

/**
 * Clear the given array
 */
void ArrayReset(Array *arr, void (*callback)(void *))
{
   clean(arr, callback);
   free(arr->items);
   arr->length = 0;
   arr->capacity = DEFAULT_ARR_CAPACITY;
   arr->items = calloc(DEFAULT_ARR_CAPACITY, sizeof(void *));
}

/**
 * Returns the last element in the given array
 */
void* ArrayLast(Array *arr)
{
   return arr->items[arr->length - 1];
}

/**
 * Free all memories recursively
 * Array *arr: the array pointer that needs to be freed
 */
void DeleteArray(Array *arr)
{
   free(arr->items);
   free(arr);
}

/**
 * Free all memories recursively, including its items.
 * Array *arr: the array pointer that needs to be freed
 * void (void*): the callback function to free item.
 */
void DeleteArrayAndItsItems(Array *arr, void (*callback)(void *))
{
   clean(arr, callback);
   DeleteArray(arr);
}

/**
 * Append the item to the array
 * Array *arr: the array pointer
 * void *item: pointer of the item
 * returns: 0 by default
 */
int ArrayAppend(Array *arr, void *item)
{
   if (arr->length >= arr->capacity) // if current lenght exceeds capacity
   {
      int newCapacity = arr->capacity << 1; // resize the array and double capacity
      void **newItems = calloc(newCapacity, sizeof(void *));
      for (int i = 0; i < arr->capacity; i++)
      {
         newItems[i] = arr->items[i]; // copy everything from old array into the new one
      }
      free(arr->items); // free the old one
      arr->capacity = newCapacity;
      arr->items = newItems;
   }
   arr->items[arr->length] = item; // append the item
   arr->length += 1;               // add 1 to the length
   return 0;
}

/**
* Get an array item by index
* Array *arr: the array pointer
* int index: the index
* returns: pointer of the item from arr[index]
*/
void *ArrayGet(Array *arr, int index)
{
   if (index >= arr->length) // check index validity
   {
      return NULL;
   }
   return arr->items[index]; // get item
}

/**
* Pop the last item from the array
* Array *arr: the array pointer
* returns: pointer of the last item
*/
void *ArrayPop(Array *arr)
{
   if (arr->length == 0)
   {
      return NULL;
   }
   arr->length--;
   return arr->items[arr->length];
}