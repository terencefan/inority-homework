#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

const int DEFAULT_ARR_CAPACITY = 32;

int ArrayAppend(Array *arr, void *item);
Array *ArrayConcat(Array *arr, Array *other);
void *ArrayPop(Array *arr);
void ArraySwap(Array *arr, int i, int j);
void *ArrayGet(Array *arr, int index);

Array *NewArray()
{
   Array *arr = calloc(1, sizeof(Array));
   arr->capacity = DEFAULT_ARR_CAPACITY;
   arr->length = 0;
   arr->items = calloc(DEFAULT_ARR_CAPACITY, sizeof(void *));
   arr->Get = ArrayGet;
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
 */
void DeleteArray(Array *arr)
{
   for (int i = 0; i < arr->length; i++)
   {
      free(arr->items[i]);
   }
   free(arr->items);
   free(arr);
}

int ArrayAppend(Array *arr, void *item)
{
   if (arr->length >= arr->capacity)
   {
      int newCapacity = arr->capacity << 1;
      void **newItems = calloc(newCapacity, sizeof(void *));
      for (int i = 0; i < arr->capacity; i++)
      {
         newItems[i] = arr->items[i];
      }
      free(arr->items);
      arr->capacity = newCapacity;
      arr->items = newItems;
   }
   arr->items[arr->length] = item;
   arr->length += 1;
   return 0;
}

void *ArrayGet(Array *arr, int index)
{
   if (index >= arr->length)
   {
      return NULL;
   }
   return arr->items[index];
}

void *ArrayPop(Array *arr)
{
   if (arr->length == 0)
   {
      return NULL;
   }
   arr->length--;
   return arr->items[arr->length];
}