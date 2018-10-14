#ifndef _MASC_ITER_H_
#define _MASC_ITER_H_

#include <stdbool.h>

#include <masc/object.h>


typedef struct Iterable Iterable;

typedef struct {
    Object;
    Iterable *iterable;
    void *priv;
} Iter;


extern const class *IterCls;


void *next(Iter *self);
 
void iter_del_obj(Iter *self);
bool iter_is_last(Iter *self);
int iter_get_idx(Iter *self);
const char *iter_get_key(Iter *self);


#endif /* _MASC_ITER_H_ */
