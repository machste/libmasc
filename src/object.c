#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <masc/object.h>
#include <masc/cstr.h>


void object_init(Object *self, const Class *cls)
{
    self->cls = cls;
}

static void _vinit(void *self, va_list _)
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

size_t object_to_cstr(Object *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s at %p>", name_of(self), self);
}


static Class _ObjectCls = {
    .name = "Object",
    .size = sizeof(Object),
    .vinit = _vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .len = (len_cb)object_len,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)object_to_cstr,
    .to_cstr = (to_cstr_cb)object_to_cstr,
    .iter_init = NULL,
};

const Class *ObjectCls = &_ObjectCls;
