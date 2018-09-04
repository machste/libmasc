#ifndef _MASC_ITER_H_
#define _MASC_ITER_H_

#include <stdbool.h>

#include <masc/object.h>


typedef struct Iter Iter;

typedef void *(*iter_next_cb)(Iter *self, void *iterable);
typedef bool (*iter_is_last_cb)(Iter *self, void *iterable);
typedef int (*iter_get_idx_cb)(Iter *self, void *iterable);
typedef const char *(*iter_get_key_cb)(Iter *self, void *iterable);

typedef void (*iter_free_priv_cb)(void *priv);

struct Iter {
    Object obj;
    void *iterable;
    void *priv;
    iter_next_cb next;
    iter_is_last_cb is_last;
    iter_get_idx_cb get_idx;
    iter_get_key_cb get_key;
    iter_free_priv_cb free_priv;
};


extern const Class *IterCls;


void *next(Iter *self);

bool iter_is_last(Iter *self);
int iter_get_idx(Iter *self);
const char *iter_get_key(Iter *self);

#endif /* _MASC_ITER_H_ */
