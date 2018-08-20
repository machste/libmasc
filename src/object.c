#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <masc/object.h>
#include <masc/cstr.h>


static const char *null_as_cstr = "null";


void object_init(Object *self, const Class *cls)
{
    self->cls = cls;
}

void object_init_copy(Object *self, Object *other)
{
    memcpy(self, other, class_of(other)->size);
}

void object_destroy(Object *self)
{
}

size_t object_to_cstr(Object *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s at %p>", name_of(self), self);
}

const Class *class_of(void *self)
{
    if (self != NULL) {
        return ((Object *)self)->cls;
    } else {
        return NULL;
    }
}

const char *name_of(void *self)
{
    if (self != NULL) {
        return class_of(self)->name;
    } else {
        return null_as_cstr;
    }
}

void *__new__(const Class *cls, ...)
{
    va_list va;
    void *self = malloc(cls->size);
    va_start(va, cls);
    cls->vinit(self, va);
    va_end(va);
    return self;
}

void __init__(const Class *cls, void *self, ...)
{
    va_list va;
    va_start(va, self);
    cls->vinit(self, va);
    va_end(va);
}

void *new_copy(void *other)
{
    const Class *cls = class_of(other);
    void *self = malloc(cls->size);
    cls->init_copy(self, other);
    return self;
}

void init_copy(void *self, void *other)
{
    const Class *cls = class_of(other);
    cls->init_copy(self, other);
}

void destroy(void *self)
{
    if (self == NULL) {
        return;
    }
    class_of(self)->destroy(self);
    
}

void delete(void *self)
{
    if (self == NULL) {
        return;
    }
    class_of(self)->destroy(self);
    free(self);
}

size_t repr(void *self, char *cstr, size_t size)
{
    if (self != NULL && class_of(self) != NULL) {
        return class_of(self)->repr(self, cstr, size);
    } else {
        cstr_copy(cstr, null_as_cstr, size);
        return 4;
    }
}

size_t to_cstr(void *self, char *cstr, size_t size)
{
    if (self != NULL && class_of(self) != NULL) {
        return class_of(self)->to_cstr(self, cstr, size);
    } else {
        return cstr_copy(cstr, null_as_cstr, size);
    }
}


static void _vinit(void *self, va_list _)
{
    object_init(self, ObjectCls);
}

static Class _ObjectCls = {
    .name = "Object",
    .size = sizeof(Object),
    .vinit = _vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .repr = (repr_cb)object_to_cstr,
    .to_cstr = (to_cstr_cb)object_to_cstr,
};

const void *ObjectCls = &_ObjectCls;
