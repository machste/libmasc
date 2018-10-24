#include <masc/iterable.h>


Iter *iterable_iter(Iterable *self)
{
    return new(Iter, self);
}
