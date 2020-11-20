#define DELETE_VAL 0x1

#include "array.h"

typedef struct _Map Map;
typedef struct _MapSlot MapSlot;
typedef struct _MapEntity MapEntity;
typedef struct _MapIterator MapIterator;

struct _Map // define the map struct
{
    void *(*Get)(Map *map, const char *key); // define built-in functions
    int (*Add)(Map *map, const char *key, void *val);
    int (*Count)(Map *map);
};

struct _MapEntity // define the struct for map item
{
    const char *key;
    void *val;
};

struct _MapIterator // define the struct fot map iterator
{
    MapEntity *current;
    void (*Next)(MapIterator *iter);
    void (*Reset)(MapIterator *iter);
};

/**
 * New a map struct
 * returns the address of the map
 */
Map *NewMap();

/**
 * Delete the map and free the memories
 * Map *map: address of the map
 * int option: 1 if delete map values at the same time, 0 otherwise
 */
void DeleteMap(Map *map, int option);

/**
 * Create a new iterator for the map
 * Map *map: address of the map
 * returns address of the map iterator
 */
MapIterator *NewMapIterator(Map *map);

Array *GetMapIteratorArray(MapIterator *iter);

/**
 * Delete map iterator
 * MapIterator *iter: address of map iterator
 */
void DeleteMapIterator(MapIterator *iter);
