#ifndef _MASC_ITER_H_
#define _MASC_ITER_H_

#include <masc/object.h>


typedef struct Iter Iter;

typedef void *(*next_cb)(Iter *self, void *iterable);

struct Iter {
    Object obj;
    size_t index;
    const char *key;
    void *iterable;
    next_cb next;
    void *ptr;
};


void iter_init(Iter *self, void *iterable, next_cb next, void *ptr,
        size_t index, const char *key);

void *next(Iter *self);

#endif /* _MASC_ITER_H_ */
