typedef struct _LinkedList LinkedList;
typedef struct _LinkedListIterator LinkedListIterator;

struct _LinkedList
{
    void *(*First)(LinkedList *);
    void *(*Last)(LinkedList *);
    void (*Add)(LinkedList *, void *);
    int (*Count)(LinkedList *);
};

struct _LinkedListIterator
{
    int (*End)(LinkedListIterator *);
    void *(*Current)(LinkedListIterator *);
    void (*Next)(LinkedListIterator *);
};

LinkedList *NewLinkedList();

LinkedListIterator *NewLinkedListIterator(LinkedList *);

void DeleteLinkedList(LinkedList *);

void DeleteLinkedListIterator(LinkedListIterator *);