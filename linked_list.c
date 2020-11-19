#include <stdio.h>
#include <stdlib.h>

#include "linked_list.h"

#define INNER(list) (LinkedListInner *)((char*)list - sizeof(LinkedListInner))
#define INNER_ITER(iter) (LinkedListIteratorInner *)((char*)iter - sizeof(LinkedListIteratorInner))

typedef struct _LinkedListNode LinkedListNode;

struct _LinkedListNode
{
    void *data;
    LinkedListNode *next;
};

typedef struct _LinkedListInner
{
    LinkedListNode dummyHead;
    LinkedListNode *tail;
    int count;
} LinkedListInner;

typedef struct _LinkedListIteratorInner
{
    LinkedListNode *current;
} LinkedListIteratorInner;

void *first(LinkedList *list)
{
    LinkedListInner *inner = INNER(list);
    if (inner->count == 0)
    {
        return NULL;
    }
    return inner->dummyHead.next->data;
}

void *last(LinkedList *list)
{
    LinkedListInner *inner = INNER(list);
    return inner->tail->data;
}

void add(LinkedList *list, void *data)
{
    LinkedListInner *inner = INNER(list);
    LinkedListNode *node = calloc(1, sizeof(LinkedListNode));
    node->data = data;
    inner->tail->next = node;
    inner->tail = node;
    inner->count++;
}

int LinkedListCount(LinkedList *list)
{
    printf("linked list count\n");
    LinkedListInner *inner = INNER(list);
    return inner->count;
}

LinkedList *NewLinkedList()
{
    LinkedListInner *inner = calloc(1, sizeof(LinkedListInner) + sizeof(LinkedList));
    inner->count = 0;
    inner->dummyHead.data = NULL;
    inner->dummyHead.next = NULL;
    inner->tail = &inner->dummyHead;

    LinkedList *list = (LinkedList *)((char *)inner + sizeof(LinkedListInner));
    list->First = first;
    list->Last = last;
    list->Add = add;
    list->Count = LinkedListCount;

    return list;
}

void *current(LinkedListIterator *iter)
{
    LinkedListIteratorInner *inner = INNER_ITER(iter);
    return NULL == inner->current ? NULL : inner->current->data;
}

int end(LinkedListIterator *iter)
{
    LinkedListIteratorInner *inner = INNER_ITER(iter);
    return NULL == inner->current;
}

void LinkedListIteratorNext(LinkedListIterator *iter)
{
    LinkedListIteratorInner *inner = INNER_ITER(iter);
    if (NULL != inner->current)
    {
        inner->current = inner->current->next;
    }
}

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

void DeleteLinkedList(LinkedList *list)
{
    LinkedListIterator *iter = NewLinkedListIterator(list);
    LinkedListIteratorInner *iterInner = INNER_ITER(iter);
    LinkedListInner *inner = INNER(list);

    while (!iter->End(iter))
    {
        LinkedListNode *node = iterInner->current;
        iter->Next(iter);
        free(node);
    }
    free(inner);

    DeleteLinkedListIterator(iter);
}

void DeleteLinkedListIterator(LinkedListIterator *iter)
{
    free(INNER_ITER(iter));
}

#ifdef LINKEDLIST
void main()
{
    LinkedList* list = NewLinkedList();
    
    for (int i = 0; i < 100; i++) {
        int *value = malloc(sizeof(int));
        *value = i;
        list->Add(list, value);
    }

    for (LinkedListIterator *iter = NewLinkedListIterator(list); !iter->End(iter); iter->Next(iter))
    {
        int *value = (int *)iter->Current(iter);
        printf("%d", *value);
    }

    DeleteLinkedList(list);
}
#endif