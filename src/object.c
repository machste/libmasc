#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <masc/object.h>
#include <masc/cstr.h>


void object_init(Object *self, const class *cls)
{
    self->cls = cls;
}

static void _vinit(Object *self, va_list _)
{
    object_init(self, ObjectCls);
}

void object_init_copy(Object *self, const Object *other)
{
    memcpy(self, other, class_of(other)->size);
}

void object_destroy(Object *self)
{
}

size_t object_len(const Object *self)
{
    return 0;
}

int object_cmp(const Object *self, const Object *other)
{
    if (self == other) {
        return 0;
    } else if (self > other) {
        return 1;
    } else {
        return -1;
    }
}

size_t object_to_cstr(const Object *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s at %p>", name_of(self), self);
}


static class _ObjectCls = {
    .name = "Object",
    .size = sizeof(Object),
    .vinit = _vinit,
    .init_copy = object_init_copy,
    .destroy = object_destroy,
    .len = object_len,
    .cmp = object_cmp,
    .repr = object_to_cstr,
    .to_cstr = object_to_cstr,
    .iter_init = NULL,
};

const class *ObjectCls = &_ObjectCls;
