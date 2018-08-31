#ifndef _MASC_ITER_H_
#define _MASC_ITER_H_

#include <stdbool.h>

#include <masc/object.h>


typedef struct Iter Iter;

typedef void *(*next_cb)(Iter *self, void *iterable);
typedef bool (*is_last_cb)(Iter *self, void *iterable);

struct Iter {
    Object obj;
    size_t index;
    const char *key;
    void *iterable;
    next_cb next;
    is_last_cb is_last;
    void *ptr;
};


void iter_init(Iter *self, void *iterable, next_cb next, is_last_cb is_last,
        void *ptr, size_t index, const char *key);

void *next(Iter *self);
bool is_last(Iter *self);

#endif /* _MASC_ITER_H_ */
