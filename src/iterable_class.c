#include <masc/iterable_class.h>
#include <masc/none.h>
#include <masc/iter.h>


const iterable_class *iterable_class_of(const Object *self)
{
    if (isinstance(self, Iterable)) {
        return (iterable_class *)self->cls;
    } else {
        return NULL;
    }
}

size_t len(const Object *self)
{
    const iterable_class *iter_cls = iterable_class_of(self);
    if (is_none(iter_cls)) {
        return 0;
    }
    return iter_cls->len((Iterable *)self);
}

void filter(Iterable *self, filter_cb cb)
{
    Iter i = init(Iter, self);
    for (void *obj = next(&i); obj != NULL; obj = next(&i)) {
        if (!cb(obj)) {
            iter_del_obj(&i);
        }
    }
    destroy(&i);
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static iterable_class _IterableCls = {
    .name = "Interable",
    .size = sizeof(Iterable),
    .super = NULL,
    .init_class = _init_class,
    // This is an abstract class
    .vinit = NULL,
    .init_copy = NULL,
    .destroy = NULL,
    .cmp = NULL,
    .repr = NULL,
    .to_cstr = NULL,
    // Interable Class
    .len = NULL,
    .new_priv = NULL,
    .next = NULL,
    .del_obj = NULL,
    .is_last = NULL,
    .get_idx = NULL,
    .get_key = NULL,
    .delete_priv = NULL,
};

const iterable_class *IterableCls = &_IterableCls;
