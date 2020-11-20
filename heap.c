#include <stdio.h>
#include <stdlib.h>

#include "heap.h"

#define PARENT(x) (x >> 1)                  // define the parent, left child, right child node
#define LCHILD(x) (((x + 1) << 1) - 1)
#define RCHILD(x) ((x + 1) << 1)


/**
 * Swap two heap items
 * Heap *heap: heap
 * int i,j: 2 indexes for swapping
 * */

void swap(Heap *heap, int i, int j)
{
  HeapItem **items = heap->items;
  HeapItem *temp;
  temp = items[j];
  items[j] = items[i];
  items[i] = temp;
}

/**
 * heapify, bottomup
 * Heap *heap: heap pointer
 * int i: index of the item
 * */
void bottomup(Heap *heap, int i)
{
  if (i == 0)
    return;

  HeapItem **items = heap->items;
  int parent = PARENT(i);

  if (items[parent]->weight > items[i]->weight)
  {
    swap(heap, parent, i);
    bottomup(heap, parent);
  }

}

/**
 * heapify, topdown
 * Heap *heap: heap pointer
 * int i: i of the item
 * */
void topdown(Heap *heap, int i)
{
  int size = heap->size;

  if (i > size)
    return;

  int lchild = LCHILD(i);
  int rchild = RCHILD(i);

  HeapItem **items = heap->items;

  if (rchild < size)
  {
    if (items[rchild]->weight <= items[lchild]->weight && items[rchild]->weight < items[i]->weight)
    {
      swap(heap, rchild, i);
      topdown(heap, rchild);
      return;

    }
  }

  if (lchild < size)
  {
    if ((rchild >= size || items[lchild]->weight <= items[rchild]->weight) && items[lchild]->weight < items[i]->weight)
    {
      swap(heap, lchild, i);
      topdown(heap, lchild);
      return;
    }
  }
}

/**
 * get heap size
 * Heap *heap: heap pointer
 * */
int HeapSize(Heap *heap)
{
  return heap->size;
}

/**
 * get the item on top of the heap
 * Heap *heap: heap pointer
 * int *weight: stores the weight of the item at the top
 * returns: pointer of the item on top of the heap
 * */
void *HeapTop(Heap *heap, int *weight)
{

  if (heap->size == 0)
  {
    return NULL;
  }

  HeapItem *item = heap->items[0];
  *weight = item->weight;
  return item->data;
}

/**
 * push an item into the heap
 * Heap *heap: heap pointer
 * int *eight: the weight of the item
 * void *data: pointer of the item that needs to be pushed
 * */
void HeapPush(Heap *heap, int weight, void *data)
{
  if (heap->size < heap->capacity)
  {
    HeapItem *item = calloc(1, sizeof(HeapItem));
    item->weight = weight;
    item->data = data;
    heap->items[heap->size] = item;
    heap->size++;
    bottomup(heap, heap->size - 1);
    return;
  }

  HeapItem *topItem = heap->items[0];
  if (heap->Compare(weight, topItem->weight, data, topItem->data) > 0)
  {
    HeapItem *item = calloc(1, sizeof(HeapItem));
    item->weight = weight;
    item->data = data;
    heap->items[0] = item;
    topdown(heap, 0);
  }
    HeapItem *item = calloc(1, sizeof(HeapItem));   // alloc memory for new heap item
    item->weight = weight;  // set properties
    item->data = data;

    if (heap->size < heap->capacity)    // if size doesn't reach the capacity
    {
        heap->items[heap->size] = item; // push item into the heap and heapify
        heap->size++;
        bottomup(heap, heap->size - 1);
    }
    else if (weight > heap->items[0]->weight)   // if size reaches the capacity but current item has a higher weight
    {                                           // than the top item in heap
        heap->items[0] = item;                  // update the item
        topdown(heap, 0);                // heapify
    }
}


/**
 * push an item into the heap
 * Heap *heap: heap pointer
 * int *eight: the weight of the item
 * void *data: pointer of the item that needs to be pushed
 * */
void *HeapPop(Heap *heap, int *weight)
{
  if (heap->size == 0)
  {
    return NULL;
  }

  HeapItem *item = heap->items[0];
  *weight = item->weight;
  void *result = item->data;
  free(item);

  swap(heap, 0, heap->size - 1);
  heap->size--;

  if (heap->size > 0)
    topdown(heap, 0);

  return result;
}

Heap *NewHeap(int capacity)
{
  Heap *heap = calloc(1, sizeof(Heap));
  heap->capacity = capacity;
  heap->size = 0;
  heap->items = calloc(capacity + 1, sizeof(void *));
  heap->Top = HeapTop;
  heap->Push = HeapPush;
  heap->Pop = HeapPop;
  heap->Size = HeapSize;
  return heap;
}

int CompareForMinHeap(int w1, int w2, void *data1, void *data2)
{
  return (w1 - w2) < 0 ? -1 : (w1 - w2) > 0 ? 1 : 0;
}

int CompareForMaxHeap(int w1, int w2, void *data1, void *data2)
{
  return (w1 - w2) > 0 ? -1 : (w1 - w2) < 0 ? 1 : 0;
}

/**
 * create a new heap
 * int capacity: maximum number of elements of the heap
 * returns: pointer of the new heap
 * */
Heap *NewMinHeap(int capacity)
{
  Heap *heap = NewHeap(capacity);
  heap->Compare = CompareForMinHeap;
  return heap;
}

Heap *NewMaxHeap(int capacity)
{
  Heap *heap = NewHeap(capacity);
  heap->Compare = CompareForMaxHeap;
  return heap;
}

/**
 * free the heap & heap items
 * Heap *heap: address of the heap
 * */
void DeleteHeap(Heap *heap)
{
  free(heap->items);
  free(heap);
}