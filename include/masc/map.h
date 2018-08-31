#ifndef _MASC_MAP_H_
#define _MASC_MAP_H_

#include <masc/object.h>
#include <masc/iter.h>


typedef struct MapNode MapNode;

typedef struct {
    Object obj;
    MapNode *node;
} Map;


extern const Class *MapCls;


Map *map_new(void);
void map_init(Map *self);

Map *map_new_copy(Map *other);
void map_init_copy(Map *self, Map *other);

void map_destroy(Map *self);
void map_delete(Map *self);

void *map_get(Map *self, const char *key);
void map_set(Map *self, const char *key, void *value);

Iter map_iter(Map *self);

void map_for_each(Map *self, void (*value_cb)(void *));
void map_for_each_kv(Map *self, void (*kv_cb)(const char *, void *));

size_t map_to_cstr(Map *self, char *cstr, size_t size);


#endif /* _MASC_MAP_H_ */
