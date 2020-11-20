#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "array.h"
#include "linked_list.h"

#define MOD 1000000007

//=============helper functions, sepc are at below=============
void *_innerGet(Map *map, const char *key, LinkedList **slot);
void *MapGet(Map *map, const char *key);
int MapAdd(Map *map, const char *key, void *val);
int MapCount(Map *map);

typedef struct _MapInner    // define the struct inside the map
{
    int entityCount;
    int slotCount;
    LinkedList **slots;     // use linkedlist to resolve hash conflict
} MapInner;

typedef struct _MapIteratorInner    // inner iterator
{
    int index;
    Array *array;
} MapIteratorInner;

#define INNER(map) (MapInner *)((char *)map - sizeof(MapInner))
#define INNER_ITER(iter) (MapIteratorInner *)((char *)iter - sizeof(MapIteratorInner))

/**
 * Hash function
 * const char *word: words to hash
 * returns: hash value
 * */
long hash(const char *word)
{
    long v = 0;
    for (int index = 0; word[index] != '\0'; index++)
    {
        v *= 31;
        v += word[index];
        v %= MOD;
    }
    return v;
}

/**
 * double the map size
 * Map *map: address of the map
 * */
void map_resize(Map *map)
{
    MapInner *inner = INNER(map);
    if (inner->entityCount < inner->slotCount)
        return;

    int newSlotCount = inner->slotCount << 1;   // get new slot number
    LinkedList **slots = inner->slots;
    int slotCount = inner->slotCount;           // update the slot number
    inner->slots = calloc(newSlotCount, sizeof(void *));    // alloc new slots
    inner->slotCount = newSlotCount;

    for (int i = 0; i < slotCount; i++)
    {
        LinkedList *slot = slots[i];
        if (slot == NULL)
            continue;
        LinkedListIterator *iter = NewLinkedListIterator(slot);

        for (; !iter->End(iter); iter->Next(iter))          // iterate throught the old slot
        {                                                   // copy everything into the new slot
            MapEntity *entity = iter->Current(iter);
            LinkedList *target;
            _innerGet(map, entity->key, &target);
            target->Add(target, entity);
        }

        DeleteLinkedList(slot);                         // free the old slot
        DeleteLinkedListIterator(iter);
    }

    free(slots);
}

/**
 * create a new map entity
 * const char *key: string key
 * void *val: value
 * returns: pointer of the new entity
 * */
MapEntity *NewMapEntity(const char *key, void *val)
{
    MapEntity *entity = calloc(1, sizeof(MapEntity));   // alloc memory

    int len = strlen(key);
    char *mapKey = calloc(1, len + 1);
    strcpy(mapKey, key);                    // copy the key
    mapKey[len] = '\0';

    entity->key = mapKey;                   // update the key & value
    entity->val = val;
    return entity;
}

/**
 * New a map struct
 * returns the address of the map
 */
Map *NewMap()
{
    MapInner *inner = calloc(1, sizeof(MapInner) + sizeof(Map));    // init slots
    inner->entityCount = 0;
    inner->slotCount = 64;
    inner->slots = calloc(inner->slotCount, sizeof(void *));

    Map *map = (Map *)((char *)inner + sizeof(MapInner));                   // init the map
    map->Get = MapGet;                                                      // set the built-in methods
    map->Add = MapAdd;
    map->Count = MapCount;
    return map;
}

/**
 * Delete the map and free the memories
 * Map *map: address of the map
 * int option: 1 if delete map values at the same time, 0 otherwise
 */
void DeleteMap(Map *map, int option)
{
    MapInner *inner = INNER(map);   // get the map slot
    for (int slotIndex = 0; slotIndex < inner->slotCount; slotIndex++)
    {
        LinkedList *slot = inner->slots[slotIndex];
        if (slot == NULL)
            continue;

        LinkedListIterator *iter = NewLinkedListIterator(slot);
        for (; !iter->End(iter); iter->Next(iter))
        {
            MapEntity *entity = iter->Current(iter);
            if ((option & DELETE_VAL) > 0)  // if the value also needs to be freed
            {
                free(entity->val);
            }
            free(entity);
        }
        DeleteLinkedList(slot);             // free everything
        DeleteLinkedListIterator(iter);
    }
    free(inner);
}

/**
 * Built-in Get function
 * Map *map: address of the map
 * const char *key: key
 * LinkedList **slot: address of the map slots
 * returns: the address of the value
 */
void *_innerGet(Map *map, const char *key, LinkedList **slot)
{
    MapInner *inner = INNER(map);

    long h = hash(key);                                     // get hash value
    int slotIndex = h % inner->slotCount;                   // get the slot according to the hash value
    if (inner->slots[slotIndex] == NULL)
        inner->slots[slotIndex] = (void *)NewLinkedList();
    *slot = inner->slots[slotIndex];

    LinkedListIterator *iter = NewLinkedListIterator(*slot);

    for (; !iter->End(iter); iter->Next(iter))  // iterate thru the slot list to get the value
    {
        MapEntity *entity = iter->Current(iter);
        if (strcmp(key, entity->key) == 0)
            return entity->val;
    }
    return NULL;
}

/**
 * Get value for the key
 * Map *map: address of the map
 * const char *key: key
 * returns: the address of the value
 */
void *MapGet(Map *map, const char *key)
{
    LinkedList *slot;
    return _innerGet(map, key, &slot);
}

/**
 * Add key and value pair to the map
 * Map *map: address of the map
 * const char *key: key
 * void *val: the address of the value
 */
int MapAdd(Map *map, const char *key, void *val)
{
    MapInner *inner = INNER(map);
    LinkedList *slot;
    if (NULL == _innerGet(map, key, &slot))
    {
        slot->Add(slot, NewMapEntity(key, val));
        inner->entityCount++;
        map_resize(map);
        return 0;
    }
    return -1;
}

/**
 * Get size of the map
 * Map *map: address of the map
 * returns total number of map items
 */
int MapCount(Map *map)
{
    MapInner *inner = INNER(map);
    return inner->entityCount;
}

/**
 * Move map iterator to the next item
 * MapIterator *iter: map iterator
 */
void MapIteratorNext(MapIterator *iter)
{
    MapIteratorInner *inner = INNER_ITER(iter); // get the iterator
    iter->current = NULL;
    inner->index++;

    if (inner->index < inner->array->length) {  // move pointer to next item
        iter->current = inner->array->Get(inner->array, inner->index);
    }
}

/**
 * Create a new iterator for the map
 * Map *map: address of the map
 * returns address of the map iterator
 */
MapIterator *NewMapIterator(Map *map)
{
    // alloc and init the iterator
    MapIteratorInner *inner = calloc(1, sizeof(MapIteratorInner) + sizeof(MapIterator));
    inner->index = -1;
    inner->array = NewArray();
    Array *array = inner->array;

    MapInner *mapInner = INNER(map);    // get the inner struct
    for (int i = 0; i < mapInner->slotCount; i++)
    {
        LinkedList *slot = mapInner->slots[i];
        if (slot == NULL)
            continue;
        LinkedListIterator *iter = NewLinkedListIterator(slot);
        for (; !iter->End(iter); iter->Next(iter))
            array->Append(array, iter->Current(iter));
        DeleteLinkedListIterator(iter);
    }
    // alloc memory and init the iterator
    MapIterator *iter = (MapIterator *)((char *)inner + sizeof(MapIteratorInner));
    iter->current = NULL;
    iter->Next = MapIteratorNext;
    MapIteratorNext(iter);
    return iter;
}

/**
 * Delete map iterator
 * MapIterator *iter: address of map iterator
 */
void DeleteMapIterator(MapIterator *iter)
{
    free(INNER_ITER(iter));
}