#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "array.h"

#define SLOTS 128 // TODO: dynamic slots

#define MOD 1000000007

void *MapGet(Map *map, const char *key);
int MapAdd(Map *map, const char *key, void *val);
int MapCount(Map *map);

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
    Map *map = calloc(1, sizeof(Map));
    map->inner.entityCount = 0;
    map->inner.slotCount = SLOTS;
    map->inner.slots = calloc(SLOTS, sizeof(void *));
    for (int i = 0; i < SLOTS; i++)
    {
        map->inner.slots[i] = (void *)NewArray();
    }

    map->Get = MapGet;
    map->Add = MapAdd;
    map->Count = MapCount;
    return map;
}

void DeleteMap(Map *map, int option)
{
    for (int slotIndex = 0; slotIndex < map->inner.slotCount; slotIndex++)
    {
        Array *slot = (Array *)map->inner.slots[slotIndex];
        for (int entityIndex = 0; entityIndex < slot->length; entityIndex++)
        {
            MapEntity *entity = slot->Get(slot, entityIndex);
            if ((option & DELETE_VAL) > 0)
            {
                free(entity->val);
            }
            free(entity);
        }
        free(slot);
    }
    free(map);
}

void *_innerGet(Map *map, const char *key, Array **slot)
{
    long h = hash(key);
    // printf("%s, slot: %ld\n", key, h % SLOTS);
    MapInner inner = map->inner;
    *slot = (Array *)inner.slots[h % inner.slotCount];

    for (int index = 0; index < (*slot)->length; index++)
    {
        MapEntity *entity = (MapEntity *)(*slot)->Get(*slot, index);
        if (strcmp(key, entity->key) == 0)
        {
            return entity->val;
        }
    }
    return NULL;
}

void *MapGet(Map *map, const char *key)
{
    Array *slot;
    return _innerGet(map, key, &slot);
}

int MapAdd(Map *map, const char *key, void *val)
{
    Array *slot;
    if (NULL == _innerGet(map, key, &slot))
    {
        slot->Append(slot, NewMapEntity(key, val));
        map->inner.entityCount++;
        return 0;
    }
    return -1;
}

int MapCount(Map *map)
{
    return map->inner.entityCount;
}

void next(MapIterator *iter)
{
    iter->current = NULL;
    Map *map = iter->map;
    iter->entityIndex++; // move forward.
    // printf("(%d, %d)\n", iter->slotIndex, iter->entityIndex);

    while (iter->slotIndex < map->inner.slotCount)
    {
        Array *slot = (Array *)map->inner.slots[iter->slotIndex];
        if (iter->entityIndex < slot->length)
        {
            // TODO: check entity deleted status if MapDel had been introduced in the future.
            iter->current = slot->Get(slot, iter->entityIndex);
            return;
        }

        // reset entity index and move the slot index forward.
        iter->entityIndex = 0;
        iter->slotIndex++;
    }
    return;
}

MapIterator *NewMapIterator(Map *map)
{
    MapIterator *iter = calloc(1, sizeof(MapIterator));
    iter->slotIndex = 0;
    iter->entityIndex = -1; // entity may start from the position (0, 0)
    iter->map = map;
    iter->current = NULL;
    iter->Next = next;
    next(iter);
    return iter;
}

void DeleteMapIterator(MapIterator *iter)
{
    free(iter);
}