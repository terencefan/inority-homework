#define DELETE_VAL 0x1

#include "array.h"

typedef struct _Map Map;
typedef struct _MapSlot MapSlot;
typedef struct _MapEntity MapEntity;
typedef struct _MapIterator MapIterator;

struct _Map
{
    void *(*Get)(Map *map, const char *key);
    int (*Add)(Map *map, const char *key, void *val);
    int (*Count)(Map *map);
};

struct _MapEntity
{
    const char *key;
    void *val;
};

struct _MapIterator
{
    MapEntity *current;
    void (*Next)(MapIterator *iter);
    void (*Reset)(MapIterator *iter);
};

Map *NewMap();

void DeleteMap(Map *, int);

MapIterator *NewMapIterator(Map *);

Array *GetMapIteratorArray(MapIterator *iter);

void DeleteMapIterator(MapIterator *);