#include <stdio.h>
#include <stdlib.h>

#include "heap.h"

#define PARENT(x) (x >> 1)
#define LCHILD(x) (((x + 1) << 1) - 1)
#define RCHILD(x) ((x + 1) << 1)

void pheap(Heap* heap) {
    printf("heap: ");
    for (int i = 0; i < heap->size; i++) {
        HeapItem* item = heap->items[i];
        printf("%d ", item->weight);
    }
    printf("\n");
}

void swap(Heap* heap, int i, int j) {
    HeapItem **items = heap->items;
    HeapItem *temp;
    temp = items[j];
    items[j] = items[i];
    items[i] = temp;
}

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

void topdown(Heap* heap, int i) {
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

int HeapSize(Heap *heap) {
    return heap->size;
}

void *HeapTop(Heap *heap, int *weight)
{
    if (heap->size == 0)
    {
        return NULL;
    }
    HeapItem* item = heap->items[0];
    *weight = item->weight;
    return item->data;
}

void HeapPush(Heap *heap, int weight, void *data)
{
    HeapItem *item = calloc(1, sizeof(HeapItem));
    item->weight = weight;
    item->data = data;

    if (heap->size < heap->capacity)
    {
        heap->items[heap->size] = item;
        heap->size++;
        bottomup(heap, heap->size - 1);
    }
    else if (weight > heap->items[0]->weight)
    {
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
    void* result = item->data;

    swap(heap, 0, heap->size - 1);
    heap->size--;
    if (heap->size > 0)
        topdown(heap, 0);
    return result;
}

Heap *NewMinHeap(int capacity)
{
    Heap *heap = calloc(1, sizeof(Heap));
    heap->capacity = capacity;
    heap->size = 0;
    heap->items = calloc(capacity + 1, sizeof(void*));
    heap->Top = HeapTop;
    heap->Push = HeapPush;
    heap->Pop = HeapPop;
    heap->Size = HeapSize;
    return heap;
}

void DeleteHeap(Heap *heap)
{
    free(heap->items);
    free(heap);
}