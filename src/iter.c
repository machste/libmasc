#include <masc/iter.h>


const void *IterCls;


void iter_init(Iter *self, void *iterable, next_cb next, void *ptr,
        size_t index, const char *key)
{
    object_init(&self->obj, IterCls);
    self->iterable = iterable;
    self->next = next;
    self->ptr = ptr;
    self->index = index;
    self->key = key;
}

void *next(Iter *self)
{
    if (self->next != NULL) {
        return self->next(self, self->iterable);
    } else {
        return NULL;
    }
}


static void _vinit(Iter *self, va_list va)
{
    object_init(&self->obj, IterCls);
    self->iterable = va_arg(va, void *);
    self->next = va_arg(va, next_cb);
    self->ptr = va_arg(va, void *);
    self->index = va_arg(va, size_t);
    self->key = va_arg(va, const char *);
}

static Class _IterCls = {
    .name = "Iter",
    .size = sizeof(Iter),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .repr = (repr_cb)object_to_cstr,
    .to_cstr = (to_cstr_cb)object_to_cstr,
};

const void *IterCls = &_IterCls;
