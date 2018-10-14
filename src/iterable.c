#include <masc/iterable.h>


const iterable_class *iterable_class_of(const Iterable *self)
{
    if (self != NULL) {
        return (iterable_class *)self->cls;
    } else {
        return NULL;
    }
}

void filter(Iterable *iterable, filter_cb cb)
{
    Iter i = init(Iter, iterable);
    for (void *obj = next(&i); obj != NULL; obj = next(&i)) {
        if (!cb(obj)) {
            iter_del_obj(&i);
        }
    }
    destroy(&i);
}
