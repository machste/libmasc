#include <masc/iter.h>


void iter_init(Iter *self, void *iterable)
{
    object_init(&self->obj, IterCls);
    self->iterable = iterable;
    if (iterable != NULL && class_of(iterable) != NULL
            && class_of(iterable)->iter_init != NULL) {
        class_of(iterable)->iter_init(iterable, self);
    } else {
        self->priv = NULL;
        self->next = NULL;
        self->is_last = NULL;
        self->free_priv = NULL;
    }
}

static void _vinit(Iter *self, va_list va)
{
    void *iterable = va_arg(va, void *);
    iter_init(self, iterable);
}

static void _init_copy(Iter *self, const Iter *other)
{
    // TBD
}

static void _destroy(Iter *self)
{
    if(self != NULL && self->free_priv != NULL) {
        self->free_priv(self->priv);
    }
}

void *next(Iter *self)
{
    if (self != NULL && self->next != NULL) {
        return self->next(self, self->iterable);
    } else {
        return NULL;
    }
}

void iter_del_obj(Iter *self)
{
    if (self != NULL && self->del_obj != NULL) {
        self->del_obj(self, self->iterable);
    }   
}

bool iter_is_last(Iter *self)
{
    if (self != NULL && self->is_last != NULL) {
        return self->is_last(self, self->iterable);
    } else {
        return false;
    }
}

int iter_get_idx(Iter *self)
{
    if (self != NULL && self->get_idx != NULL) {
        return self->get_idx(self, self->iterable);
    } else {
        return 0;
    }
}

const char *iter_get_key(Iter *self)
{
    if (self != NULL && self->get_key != NULL) {
        return self->get_key(self, self->iterable);
    } else {
        return NULL;
    }
}


static Class _IterCls = {
    .name = "Iter",
    .size = sizeof(Iter),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)_init_copy,
    .destroy = (destroy_cb)_destroy,
    .repr = (repr_cb)object_to_cstr,
    .to_cstr = (to_cstr_cb)object_to_cstr,
    .iter_init = NULL,
};

const Class *IterCls = &_IterCls;
