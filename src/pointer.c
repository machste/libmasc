#include <stdio.h>

#include <masc/cstr.h>
#include <masc/pointer.h>


Pointer *pointer_new(void *ptr)
{
    Pointer *self = malloc(sizeof(Pointer));
    pointer_init(self, ptr);
    return self;
}

void pointer_init(Pointer *self, void *ptr)
{
    object_init(self, PointerCls);
    self->ptr = ptr;
}

void pointer_vinit(Pointer *self, va_list va)
{
    void *ptr = (void *)va_arg(va, void *);
    pointer_init(self, ptr);
}

void pointer_delete(Pointer *self)
{
    free(self);
}

void *pointer_get(Pointer *self)
{
    return self->ptr;
}

void pointer_set(Pointer *self, void *ptr)
{
    self->ptr = ptr;
}

int pointer_cmp(const Pointer *self, const Pointer *other)
{
    if (self->ptr == other->ptr) {
        return 0;
    } else if (self->ptr < other->ptr) {
        return -1;
    } else {
        return 0;
    }
}

size_t pointer_to_cstr(Pointer *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s ptr: %p at %p>",
            name_of(self), self->ptr, self);
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static class _PointerCls = {
    .name = "Pointer",
    .size = sizeof(Pointer),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)pointer_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .cmp = (cmp_cb)pointer_cmp,
    .repr = (repr_cb)pointer_to_cstr,
    .to_cstr = (to_cstr_cb)pointer_to_cstr,
};

const class *PointerCls = &_PointerCls;
