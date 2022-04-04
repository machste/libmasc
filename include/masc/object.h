#ifndef _MASC_OBJECT_H_
#define _MASC_OBJECT_H_

#include <stdlib.h>


typedef struct class class;
typedef struct Object {
    const class *cls;
} Object;


extern const class *ObjectCls;


void object_init(Object *self, const class *cls);

void object_init_copy(Object *self, const Object *other);

void object_destroy(Object *self);

size_t object_len(const Object *self);

int object_cmp(const Object *self, const Object *other);

size_t object_to_cstr(const Object *self, char *cstr, size_t size);

#include <masc/class.h>

#endif /* _MASC_OBJECT_H_ */
