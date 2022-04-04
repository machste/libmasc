#include <masc/iter.h>


static void _vinit(Iter *self, va_list va)
{
    object_init(self, IterCls);
    self->iterable = va_arg(va, Iterable *);
    const iterable_class *iterable_cls = iterable_class_of(self->iterable);
    if (iterable_cls != NULL && iterable_cls->new_priv != NULL) {
        self->priv = iterable_cls->new_priv(self->iterable);
    } else {
        self->priv = NULL;
    }
}

static void _init_copy(Iter *self, const Iter *other)
{
    // TBD
}

static void _destroy(Iter *self)
{
    if (self->priv != NULL) {
        const iterable_class *iterable_cls = iterable_class_of(self->iterable);
        if (iterable_cls != NULL && iterable_cls->delete_priv != NULL) {
            iterable_cls->delete_priv(self->priv);
        }
    }
}

void *next(Iter *self)
{
    const iterable_class *iterable_cls = iterable_class_of(self->iterable);
    if (iterable_cls != NULL && iterable_cls->next != NULL) {
        return iterable_cls->next(self->iterable, self->priv);
    } else {
        return NULL;
    }
}

void iter_del_obj(Iter *self)
{
    const iterable_class *iterable_cls = iterable_class_of(self->iterable);
    if (iterable_cls != NULL && iterable_cls->del_obj != NULL) {
        iterable_cls->del_obj(self->iterable, self->priv);
    }
}

bool iter_is_last(Iter *self)
{
    const iterable_class *iterable_cls = iterable_class_of(self->iterable);
    if (iterable_cls != NULL && iterable_cls->is_last != NULL) {
        return iterable_cls->is_last(self->iterable, self->priv);
    } else {
        return false;
    }
}

int iter_get_idx(Iter *self)
{
    const iterable_class *iterable_cls = iterable_class_of(self->iterable);
    if (iterable_cls != NULL && iterable_cls->get_idx != NULL) {
        return iterable_cls->get_idx(self->iterable, self->priv);
    } else {
        return -1;
    }
}

const char *iter_get_key(Iter *self)
{
    const iterable_class *iterable_cls = iterable_class_of(self->iterable);
    if (iterable_cls != NULL && iterable_cls->get_key != NULL) {
        return iterable_cls->get_key(self->iterable, self->priv);
    } else {
        return NULL;
    }
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static class _IterCls = {
    .name = "Iter",
    .size = sizeof(Iter),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)_init_copy,
    .destroy = (destroy_cb)_destroy,
    .repr = (repr_cb)object_to_cstr,
    .to_cstr = (to_cstr_cb)object_to_cstr,
};

const class *IterCls = &_IterCls;
