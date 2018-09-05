#include <stdio.h>
#include <string.h>

#include <masc/map.h>
#include <masc/iter.h>
#include <masc/math.h>
#include <masc/cstr.h>


struct MapNode {
    MapNode *next;
    char *key;
    void *value;
};


Map *map_new(void)
{
    Map *self = malloc(sizeof(Map));
    map_init(self);
    return self;
}

void map_init(Map *self)
{
    object_init(&self->obj, MapCls);
    self->node = NULL;
}

static void _vinit(Map *self, va_list va)
{
    map_init(self);
}

Map *map_new_copy(Map *other)
{
    Map *self = malloc(sizeof(Map));
    map_init_copy(self, other);
    return self;
}

void map_init_copy(Map *self, Map *other)
{
    map_init(self);
    MapNode *node = other->node;
    while (node != NULL) {
        void *new_value = NULL;
        if (node->value != NULL && class_of(node->value) != NULL) {
            new_value = new_copy(node->value);
        }
        map_set(self, node->key, new_value);
        node = node->next;
    }
}

static MapNode *mapnode_new(const char *key, void *value) {
    if (key != NULL) {
        MapNode *self = malloc(sizeof(MapNode));
        self->value = value;
        self->key = strdup(key);
        self->next = NULL;
        return self;
    }
    return NULL;
}

static void mapnode_delete(MapNode *self) {
    if (self->value != NULL && class_of(self->value) != NULL) {
        delete(self->value);
    }
    free(self->key);
    free(self);
}

void map_destroy(Map *self)
{
    MapNode *node = self->node;
    while (node != NULL) {
        MapNode *old_node = node;
        node = node->next;
        mapnode_delete(old_node);
    }
}

void map_delete(Map *self)
{
    map_destroy(self);
    free(self);
}

size_t map_len(Map *self)
{
    size_t len = 0;
    for (MapNode *n = self->node; n != NULL; n = n->next) {
        len++;
    }
    return len;
}

static MapNode *_node_by_key(Map *self, const char *key)
{
    MapNode *node = self->node;
    if (key != NULL) {
        while (node != NULL) {
            if (strcmp(node->key, key) == 0) {
                return node;
            }
            node = node->next;
        }
    }
    return NULL;
}

void *map_get(Map *self, const char *key)
{
    MapNode *node = _node_by_key(self, key);
    if (node != NULL) {
        return node->value;
    }
    return NULL;
}

static void _append(Map *self, const char *key, void *value)
{
    MapNode *new_node = mapnode_new(key, value);
    if (new_node != NULL) {
        MapNode *node = self->node;
        if (self->node == NULL) {
            self->node = new_node;
        } else {
            while (node->next != NULL) {
                node = node->next;
            }
            node->next = new_node;
        }
    }
}

void map_set(Map *self, const char *key, void *value)
{
    MapNode *node = _node_by_key(self, key);
    if (node != NULL) {
        // Delete current value
        if (node->value != NULL && class_of(node->value) != NULL) {
            delete(node->value);
        }
        node->value = value;
    } else {
        _append(self, key, value);
    }
}

static void _for_each_node(Map *self, void (*node_cb)(MapNode *))
{
    MapNode *node = self->node;
    while (node != NULL) {
        node_cb(node);
        node = node->next;
    }
}

void map_for_each(Map *self, void (*value_cb)(void *))
{
    void node_cb(MapNode *node) {
        value_cb(node->value);
    }
    _for_each_node(self, node_cb);
}

void map_for_each_kv(Map *self, void (*kv_cb)(const char *, void *))
{
    void node_cb(MapNode *node) {
        kv_cb(node->value, node->key);
    }
    _for_each_node(self, node_cb);
}

size_t map_to_cstr(Map *self, char *cstr, size_t size)
{
    int len = 0;
    len += cstr_ncopy(cstr, "{", size);
    MapNode *node = self->node;
    while (node != NULL) {
        len += snprintf(cstr + len, max(0, size - len), "\"%s\": ", node->key);
        if (node->value != NULL && class_of(node->value) != NULL) {
            len += repr(node->value, cstr + len, max(0, size - len));
        } else {
            len += cstr_ncopy(cstr + len, "null", max(0, size - len));
        }
        if (node->next != NULL) {
            len += cstr_ncopy(cstr + len, ", ", max(0, size - len));
        }
        node = node->next;
    }
    len += cstr_ncopy(cstr + len, "}", max(0, size - len));
    return len;
}


typedef struct {
    MapNode *next;
    int idx;
    const char *key;
} _IterPriv;

static void *_next(Iter *itr, Map *self)
{
    MapNode *node = ((_IterPriv *)itr->priv)->next;
    if (node != NULL) {
        ((_IterPriv *)itr->priv)->idx++;
        ((_IterPriv *)itr->priv)->key = node->key;
        ((_IterPriv *)itr->priv)->next = node->next;
        return node->value;
    }
    return NULL;

}

static bool _is_last(Iter *itr, Map *self)
{
    return ((_IterPriv *)itr->priv)->next == NULL;
}

static int _get_idx(Iter *itr, Map *self)
{
    return ((_IterPriv *)itr->priv)->idx;
}

static const char *_get_key(Iter *itr, Map *self)
{
    return ((_IterPriv *)itr->priv)->key;
}

static void _iter_init(Map *self, Iter *itr)
{
    itr->next = (iter_next_cb)_next;
    itr->del_obj = NULL;
    itr->is_last = (iter_is_last_cb)_is_last;
    itr->get_idx = (iter_get_idx_cb)_get_idx;
    itr->get_key = (iter_get_key_cb)_get_key;
    itr->priv = malloc(sizeof(_IterPriv));
    ((_IterPriv *)itr->priv)->next = self->node;
    ((_IterPriv *)itr->priv)->idx = -1;
    ((_IterPriv *)itr->priv)->key = NULL;
    itr->free_priv = free;
}


static Class _MapCls = {
    .name = "Map",
    .size = sizeof(Map),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)map_init_copy,
    .destroy = (destroy_cb)map_destroy,
    .repr = (repr_cb)map_to_cstr,
    .to_cstr = (to_cstr_cb)map_to_cstr,
    .iter_init = (iter_init_cb)_iter_init,
};

const Class *MapCls = &_MapCls;
