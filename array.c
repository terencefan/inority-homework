#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

const int DEFAULT_ARR_CAPACITY = 32;

int ArrayAppend(Array *arr, void *item);
Array *ArrayConcat(Array *arr, Array *other);
void *ArrayPop(Array *arr);
void ArraySwap(Array *arr, int i, int j);
void *ArrayGet(Array *arr, int index);

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
   arr->Get = ArrayGet; // set built-in methods
   arr->Append = ArrayAppend;
   arr->Concat = ArrayConcat;
   arr->Swap = ArraySwap;
   arr->Pop = ArrayPop;
   return arr;
}

void ArraySwap(Array *arr, int i, int j)
{
   void *temp = arr->items[i];
   arr->items[i] = arr->items[j];
   arr->items[j] = temp;
}

Array *ArrayConcat(Array *arr1, Array *arr2)
{
   for (int arrIndex = 0; arrIndex < arr2->length; arrIndex++)
      ArrayAppend(arr1, ArrayGet(arr2, arrIndex));
   free(arr2->items);
   free(arr2);
   return arr1;
}

/**
 * Free all memories recursively, including its items.
 * Array *arr: the array pointer that needs to be freed
 * int option: 1 if delete array values at the same time, 0 otherwise
 */
void DeleteArray(Array *arr)
{
   for (int i = 0; i < arr->length; i++)
      free(arr->items[i]);
   free(arr->items);
   free(arr);
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

/**
* Free the array struct but keep the list of items
* Array *arr: the array pointer
* returns: pointer of the item list
*/
void **RemoveArray(Array *arr)
{
   void **r = arr->items;
   free(arr);
   return r;
}
