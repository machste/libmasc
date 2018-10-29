#include <masc/iterable_class.h>
#include <masc/iter.h>


const iterable_class *iterable_class_of(const Iterable *self)
{
    if (self != NULL) {
        return (iterable_class *)self->cls;
    } else {
        return NULL;
    }
}

size_t len(const Iterable *self)
{
    if (self == NULL || iterable_class_of(self) == NULL) {
        return 0;
    }
    return iterable_class_of(self)->len(self);
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
