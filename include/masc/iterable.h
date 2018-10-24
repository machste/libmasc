#ifndef _MASC_ITERABLE_H_
#define _MASC_ITERABLE_H_

#include <masc/iterable_class.h>
#include <masc/object.h>
#include <masc/iter.h>


typedef struct Iterable {
    Object;
} Iterable;


Iter *iterable_iter(Iterable *self);


#endif /* _MASC_ITERABLE_H_ */
