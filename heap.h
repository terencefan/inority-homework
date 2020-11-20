typedef struct _Heap Heap;
typedef struct _HeapItem HeapItem;

struct _HeapItem            // define the struct of the item in the heap
{
    int weight;
    void *data;             // pointer of the actual item
};

struct _Heap                // define the struct of the heap
{
    int size;
    int capacity;
    HeapItem **items;       // a dynamic list of pointers of items

    int (*Compare)(int w1, int w2, void *data1, void *data2);
    void *(*Top)(Heap *heap, int *weight);
    void (*Push)(Heap *heap, int weight, void *data);
    void *(*Pop)(Heap *heap, int *weight);
    int (*Size)(Heap *heap);
};

/**
 * create a new heap
 * int capacity: maximum number of elements of the heap
 * returns: pointer of the new heap
 * */
Heap *NewMinHeap(int capacity);

/**
 * free the heap & heap items
 * Heap *heap: address of the heap
 * */
void DeleteHeap(Heap *heap);