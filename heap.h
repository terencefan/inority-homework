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

    int (*Compare)(int w1, int w2, void *data1, void *data2);

    void *(*Top)(Heap *heap, int *weight);
    void (*Push)(Heap *heap, int weight, void *data);
    void *(*Pop)(Heap *heap, int *weight);
    int (*Size)(Heap *heap);
};

Heap *NewMinHeap(int capacity);

Heap *NewMaxHeap(int capacity);

void DeleteHeap(Heap *heap);