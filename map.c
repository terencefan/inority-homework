#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "array.h"
#include "linked_list.h"

#define MOD 1000000007

void *_innerGet(Map *map, const char *key, LinkedList **slot);
void *MapGet(Map *map, const char *key);
int MapAdd(Map *map, const char *key, void *val);
int MapCount(Map *map);

typedef struct _MapInner
{
    int entityCount;
    int slotCount;
    LinkedList **slots;
} MapInner;

typedef struct _MapIteratorInner
{
    int index;
    Array *array;
} MapIteratorInner;

#define INNER(map) (MapInner *)((char *)map - sizeof(MapInner))
#define INNER_ITER(iter) (MapIteratorInner *)((char *)iter - sizeof(MapIteratorInner))

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

void map_resize(Map *map)
{
    MapInner *inner = INNER(map);
    if (inner->entityCount < inner->slotCount)
        return;
    printf("resize\n");

    int newSlotCount = inner->slotCount << 1;
    LinkedList **slots = inner->slots;
    int slotCount = inner->slotCount;
    inner->slots = calloc(newSlotCount, sizeof(void *));
    inner->slotCount = newSlotCount;

    for (int i = 0; i < slotCount; i++)
    {
        LinkedList *slot = slots[i];
        if (slot == NULL)
            continue;
        LinkedListIterator *iter = NewLinkedListIterator(slot);

        for (; !iter->End(iter); iter->Next(iter))
        {
            MapEntity *entity = iter->Current(iter);
            LinkedList *target;
            _innerGet(map, entity->key, &target);
            target->Add(target, entity);
        }

        DeleteLinkedList(slot);
        DeleteLinkedListIterator(iter);
    }

    free(slots);
}

MapEntity *NewMapEntity(const char *key, void *val)
{
    MapEntity *entity = calloc(1, sizeof(MapEntity));

    int len = strlen(key);
    char *mapKey = calloc(1, len + 1);
    strcpy(mapKey, key);
    mapKey[len] = '\0';

    entity->key = mapKey;
    entity->val = val;
    return entity;
}

/**
 * New a map struct
 */
Map *NewMap()
{
    MapInner *inner = calloc(1, sizeof(MapInner) + sizeof(Map));
    inner->entityCount = 0;
    inner->slotCount = 64;
    inner->slots = calloc(inner->slotCount, sizeof(void *));

    Map *map = (Map *)((char *)inner + sizeof(MapInner));
    map->Get = MapGet;
    map->Add = MapAdd;
    map->Count = MapCount;
    return map;
}

void DeleteMap(Map *map, int option)
{
    MapInner *inner = INNER(map);
    for (int slotIndex = 0; slotIndex < inner->slotCount; slotIndex++)
    {
        LinkedList *slot = inner->slots[slotIndex];
        if (slot == NULL)
            continue;

        LinkedListIterator *iter = NewLinkedListIterator(slot);
        for (; !iter->End(iter); iter->Next(iter))
        {
            MapEntity *entity = iter->Current(iter);
            if ((option & DELETE_VAL) > 0)
            {
                free(entity->val);
            }
            free(entity);
        }
        DeleteLinkedList(slot);
        DeleteLinkedListIterator(iter);
    }
    free(inner);
}

void *_innerGet(Map *map, const char *key, LinkedList **slot)
{
    MapInner *inner = INNER(map);

    long h = hash(key);
    int slotIndex = h % inner->slotCount;
    if (inner->slots[slotIndex] == NULL)
        inner->slots[slotIndex] = (void *)NewLinkedList();
    *slot = inner->slots[slotIndex];

    LinkedListIterator *iter = NewLinkedListIterator(*slot);

    for (; !iter->End(iter); iter->Next(iter))
    {
        MapEntity *entity = iter->Current(iter);
        if (strcmp(key, entity->key) == 0)
            return entity->val;
    }
    return NULL;
}

void *MapGet(Map *map, const char *key)
{
    LinkedList *slot;
    return _innerGet(map, key, &slot);
}

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

int MapCount(Map *map)
{
    MapInner *inner = INNER(map);
    return inner->entityCount;
}

void MapIteratorNext(MapIterator *iter)
{
    MapIteratorInner *inner = INNER_ITER(iter);
    iter->current = NULL;
    inner->index++;

    if (inner->index < inner->array->length) {
        iter->current = inner->array->Get(inner->array, inner->index);
    }
}

MapIterator *NewMapIterator(Map *map)
{
    MapIteratorInner *inner = calloc(1, sizeof(MapIteratorInner) + sizeof(MapIterator));
    inner->index = -1;
    inner->array = NewArray();
    Array *array = inner->array;

    MapInner *mapInner = INNER(map);
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

    MapIterator *iter = (MapIterator *)((char *)inner + sizeof(MapIteratorInner));
    iter->current = NULL;
    iter->Next = MapIteratorNext;
    MapIteratorNext(iter);
    return iter;
}

void DeleteMapIterator(MapIterator *iter)
{
    free(INNER_ITER(iter));
}