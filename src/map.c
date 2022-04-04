#include <stdio.h>
#include <string.h>

#include <masc/map.h>
#include <masc/str.h>
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
    object_init(self, MapCls);
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
    delete(self->value);
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

bool map_has_key(Map *self, const char *key)
{
    return _node_by_key(self, key) != NULL;
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


static MapNode *_remove_node_by_key(Map *self, const char *key)
{
    MapNode *rm_node = self->node, *prev = NULL;
    if (key != NULL) {
        while (rm_node != NULL) {
            if (strcmp(rm_node->key, key) == 0) {
                if (prev == NULL) {
                    self->node = rm_node->next;
                } else {
                    prev->next = rm_node->next;
                }
                return rm_node;
            }
            prev = rm_node;
            rm_node = rm_node->next;
        }
    }
    return NULL;
}

void *map_remove_key(Map *self, const char *key)
{
    void *value = NULL;
    MapNode *rm_node = _remove_node_by_key(self, key);
    if (rm_node != NULL) {
        value = rm_node->value;
        rm_node->value = NULL;
        mapnode_delete(rm_node);
    }
    return value;
}

bool map_delete_key(Map *self, const char *key)
{
    MapNode *rm_node = _remove_node_by_key(self, key);
    if (rm_node != NULL) {
        mapnode_delete(rm_node);
        return true;
    }
    return false;   
}

List *map_get_keys(Map *self)
{
    List *l = list_new();
    MapNode *node = self->node;
    while (node != NULL) {
        list_append(l, str_new_cstr(node->key));
        node = node->next;
    }
    return l;
}

List *map_get_values(Map *self)
{
    List *l = list_new();
    MapNode *node = self->node;
    while (node != NULL) {
        list_append(l, new_copy(node->value));
        node = node->next;
    }
    return l;
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
    long len = 0;
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
} iter_priv;

static void *_new_priv(Map *self)
{
    iter_priv *itr = malloc(sizeof(iter_priv));
    itr->next = self->node;
    itr->idx = -1;
    itr->key = NULL;
    return itr;
}

static void *_next(Map *self, iter_priv *itr)
{
    MapNode *node = itr->next;
    if (node != NULL) {
        itr->idx++;
        itr->key = node->key;
        itr->next = node->next;
        return node->value;
    }
    return NULL;

}

static void _del_obj(Map *self, iter_priv *itr)
{
    map_delete_key(self, itr->key);
    itr->idx--;
}

static bool _is_last(Map *self, iter_priv *itr)
{
    return itr->next == NULL;
}

static int _get_idx(Map *self, iter_priv *itr)
{
    return itr->idx;
}

static const char *_get_key(Map *self, iter_priv *itr)
{
    return itr->key;
}

static void _init_class(class *cls)
{
    cls->super = IterableCls;
}


static iterable_class _MapCls = {
    .name = "Map",
    .size = sizeof(Map),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)map_init_copy,
    .destroy = (destroy_cb)map_destroy,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)map_to_cstr,
    .to_cstr = (to_cstr_cb)map_to_cstr,
    // Interable Class
    .len = (len_cb)map_len,
    .new_priv = (new_priv_cb)_new_priv,
    .next = (next_cb)_next,
    .del_obj = (del_obj_cb)_del_obj,
    .is_last = (is_last_cb)_is_last,
    .get_idx = (get_idx_cb)_get_idx,
    .get_key = (get_key_cb)_get_key,
    .delete_priv = free,
};

const class *MapCls = &_MapCls;
