typedef struct _Heap Heap;
typedef struct _HeapItem HeapItem;

struct _HeapItem
{
    int weight;
    void *data;
};

struct _Heap
{
    int size;
    int capacity;
    HeapItem **items;

    void *(*Top)(Heap *heap, int *weight);
    void (*Push)(Heap *heap, int weight, void *data);
    void *(*Pop)(Heap *heap, int *weight);
    int (*Size)(Heap *heap);
};

Heap *NewMinHeap(int capacity);

void DeleteHeap(Heap *heap);