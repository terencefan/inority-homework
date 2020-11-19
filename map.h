#define DELETE_VAL 0x1

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
};

Map *NewMap();

void DeleteMap(Map *map, int option);

MapIterator *NewMapIterator(Map *map);

void DeleteMapIterator(MapIterator *iter);