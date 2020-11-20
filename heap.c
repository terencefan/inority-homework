#include <stdio.h>
#include <stdlib.h>

#include "heap.h"

#define PARENT(x) (x >> 1)
#define LCHILD(x) (((x + 1) << 1) - 1)
#define RCHILD(x) ((x + 1) << 1)

void pheap(Heap *heap)
{
  printf("heap: ");

  for (int i = 0; i < heap->size; i++)
  {
    HeapItem *item = heap->items[i];
    printf("%d ", item->weight);
  }

  printf("\n");
}

void swap(Heap *heap, int i, int j)
{
  HeapItem **items = heap->items;
  HeapItem *temp;
  temp = items[j];
  items[j] = items[i];
  items[i] = temp;
}

int InnerCompare(Heap *heap, HeapItem *item1, HeapItem *item2)
{
  int w1 = item1->weight;
  int w2 = item2->weight;
  void *data1 = item1->data;
  void *data2 = item2->data;
  return heap->Compare(w1, w2, data1, data2);
}

void bottomup(Heap *heap, int i)
{
  if (i == 0)
    return;

  HeapItem **items = heap->items;
  int parent = PARENT(i);

  if (InnerCompare(heap, items[parent], items[i]) > 0)
  {
    swap(heap, parent, i);
    bottomup(heap, parent);
  }
}

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
    if (InnerCompare(heap, items[rchild], items[lchild]) <= 0)
    {
      if (InnerCompare(heap, items[rchild], items[i]) < 0)
      {
        swap(heap, rchild, i);
        topdown(heap, rchild);
        return;
      }
    }
  }

  if (lchild < size)
  {
    if (rchild >= size || InnerCompare(heap, items[lchild], items[rchild]) <= 0)
    {
      if (InnerCompare(heap, items[lchild], items[i]) < 0)
      {
        swap(heap, lchild, i);
        topdown(heap, lchild);
        return;
      }
    }
  }
}

int HeapSize(Heap *heap)
{
  return heap->size;
}

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
}

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

void DeleteHeap(Heap *heap)
{
  free(heap->items);
  free(heap);
}