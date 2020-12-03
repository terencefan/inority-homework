#include <stdio.h>
#include <stdlib.h>

#include "linked_list.h"

#define INNER(list) (LinkedListInner *)((char *)list - sizeof(LinkedListInner))
#define INNER_ITER(iter) (LinkedListIteratorInner *)((char *)iter - sizeof(LinkedListIteratorInner))

typedef struct _LinkedListNode LinkedListNode;

struct _LinkedListNode // define the struct for node
{
    void *data;
    LinkedListNode *next;
};

typedef struct _LinkedListInner // inner struct
{
    LinkedListNode dummyHead; // always points to the head
    LinkedListNode *tail;     // points to the last node
    int count;                // linkedlist size
} LinkedListInner;

typedef struct _LinkedListIteratorInner // iterator for the linked list
{
    LinkedListNode *current;
} LinkedListIteratorInner;

/**
 * Returns the first element
 * LinkedList *list: the linked list
 * */
void *first(LinkedList *list)
{
    LinkedListInner *inner = INNER(list);
    if (inner->count == 0)
    {
        return NULL;
    }
    return inner->dummyHead.next->data;
}

/**
 * Returns the last element
 * LinkedList *list: the linked list
 * */
void *last(LinkedList *list)
{
    LinkedListInner *inner = INNER(list);
    return inner->tail->data;
}

/**
 * Add an item to the linked list
 * LinkedList *list: the linked list
 * void *data: pointer to the item
 * */
void add(LinkedList *list, void *data)
{
    LinkedListInner *inner = INNER(list);
    LinkedListNode *node = calloc(1, sizeof(LinkedListNode)); // init a node
    node->data = data;
    inner->tail->next = node;
    inner->tail = node;
    inner->count++;
}

/**
 * Returns the linked list size
 * LinkedList *list: the linked list
 * */
int LinkedListCount(LinkedList *list)
{
    LinkedListInner *inner = INNER(list);
    return inner->count;
}

/**
 * Create a new linked list
 * returns: address of the linked list
 * */
LinkedList *NewLinkedList()
{
    // init the inner struct first
    LinkedListInner *inner = calloc(1, sizeof(LinkedListInner) + sizeof(LinkedList));
    inner->count = 0;
    inner->dummyHead.data = NULL; // set up head and tail
    inner->dummyHead.next = NULL;
    inner->tail = &inner->dummyHead;

    // init the linked list it self
    LinkedList *list = (LinkedList *)((char *)inner + sizeof(LinkedListInner));
    list->First = first;
    list->Last = last;
    list->Add = add;
    list->Count = LinkedListCount;

    return list;
}

/**
 * Returns the data at the current position in the linked list
 * LinkedListIterator *iter: the linked list iterator
 * */
void *current(LinkedListIterator *iter)
{
    LinkedListIteratorInner *inner = INNER_ITER(iter);
    return NULL == inner->current ? NULL : inner->current->data;
}

/**
 * Returns 1 if reaches the end of the list, 0 otherwise
 * LinkedListIterator *iter: the linked list iterator
 * */
int end(LinkedListIterator *iter)
{
    LinkedListIteratorInner *inner = INNER_ITER(iter);
    return NULL == inner->current;
}

/**
 * Move the iterator to next node
 * LinkedListIterator *iter: the linked list iterator
 * */
void LinkedListIteratorNext(LinkedListIterator *iter)
{
    LinkedListIteratorInner *inner = INNER_ITER(iter);
    if (NULL != inner->current)
    {
        inner->current = inner->current->next;
    }
}

/**
 * Create an iterator for the linked list
 * LinkedList *list: the linked list
 * */
LinkedListIterator *NewLinkedListIterator(LinkedList *list)
{
    LinkedListInner *inner = INNER(list);
    LinkedListIteratorInner *iterInner = calloc(1, sizeof(LinkedListIteratorInner) + sizeof(LinkedListIterator));
    iterInner->current = inner->dummyHead.next;

    LinkedListIterator *iter = (LinkedListIterator *)((char *)iterInner + sizeof(LinkedListIteratorInner));
    iter->Current = current;
    iter->End = end;
    iter->Next = LinkedListIteratorNext;

    iterInner = INNER_ITER(iter);

    return iter;
}

/**
 * Delete the linked list and free its noes
 * LinkedList *list: the linked list
 * */
void DeleteLinkedList(LinkedList *list)
{
    LinkedListIterator *iter = NewLinkedListIterator(list); // get the iterator
    LinkedListIteratorInner *iterInner = INNER_ITER(iter);
    LinkedListInner *inner = INNER(list);

    while (!iter->End(iter)) // start iterating
    {
        LinkedListNode *node = iterInner->current; // free the node
        iter->Next(iter);
        free(node);
    }
    free(inner); // free everything

    DeleteLinkedListIterator(iter);
}

/**
 * Delete the linked list iterator
 * LinkedListIterator *iter: the linked list iterator
 * */
void DeleteLinkedListIterator(LinkedListIterator *iter)
{
    free(INNER_ITER(iter));
}