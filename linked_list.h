typedef struct _LinkedList LinkedList;
typedef struct _LinkedListIterator LinkedListIterator;

struct _LinkedList  // define the struct of linked list
{
    void *(*First)(LinkedList *);   // fist node
    void *(*Last)(LinkedList *);    // last node
    void (*Add)(LinkedList *, void *);  // built-in functions
    int (*Count)(LinkedList *);
};

struct _LinkedListIterator  // define struct for linked list iterator
{
    int (*End)(LinkedListIterator *);
    void *(*Current)(LinkedListIterator *);
    void (*Next)(LinkedListIterator *);
};

/**
 * Create a new linked list
 * returns: address of the linked list
 * */
LinkedList *NewLinkedList();

/**
 * Create an iterator for the linked list
 * LinkedList *list: the linked list
 * */
LinkedListIterator *NewLinkedListIterator(LinkedList *);

/**
 * Delete the linked list and free its noes
 * LinkedList *list: the linked list
 * */
void DeleteLinkedList(LinkedList *);

/**
 * Delete the linked list iterator
 * LinkedListIterator *iter: the linked list iterator
 * */
void DeleteLinkedListIterator(LinkedListIterator *);