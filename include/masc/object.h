#ifndef _MASC_OBJECT_H_
#define _MASC_OBJECT_H_

#include <stdlib.h>

#include <masc/class.h>


typedef struct {
    const Class *cls;
} Object;


extern const Class *ObjectCls;


void object_init(Object *self, const Class *cls);

void object_init_copy(Object *self, const Object *other);

void object_destroy(Object *self);

size_t object_to_cstr(Object *self, char *cstr, size_t size);


#endif /* _MASC_OBJECT_H_ */
