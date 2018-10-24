#ifndef _MASC_ITERABLE_CLASS_H_
#define _MASC_ITERABLE_CLASS_H_

#include <stdbool.h>

#include <masc/class.h>


typedef struct Iterable Iterable;

typedef size_t (*len_cb)(const Iterable *self);
typedef void *(*new_priv_cb)(Iterable *self);
typedef void *(*next_cb)(Iterable *self, void *priv);
typedef void (*del_obj_cb)(Iterable *self, void *priv);
typedef bool (*is_last_cb)(Iterable *self, void *priv);
typedef int (*get_idx_cb)(Iterable *self, void *priv);
typedef const char *(*get_key_cb)(Iterable *self, void *priv);
typedef void (*delete_priv_cb)(void *priv);

typedef struct {
    class;
    len_cb len;
    new_priv_cb new_priv;
    next_cb next;
    del_obj_cb del_obj;
    is_last_cb is_last;
    get_idx_cb get_idx;
    get_key_cb get_key;
    delete_priv_cb delete_priv;
} iterable_class;

typedef bool (*filter_cb)(Object *obj);

const iterable_class *iterable_class_of(const Iterable *self);

size_t len(const Iterable *self);

void filter(Iterable *self, filter_cb cb);


#endif /* _MASC_ITERABLE_CLASS_H_ */
