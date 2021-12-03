#include <masc/num_class.h>


const num_class *num_class_of(const Num *self)
{
    if (self != NULL) {
        return (num_class *)self->cls;
    } else {
        return NULL;
    }
}

long to_int(Num *self)
{
    const num_class *cls = num_class_of(self);
    if (cls != NULL) {
        return cls->to_int(self);
    }
    return 0;
}

double to_double(Num *self)
{
    const num_class *cls = num_class_of(self);
    if (cls != NULL) {
        return cls->to_double(self);
    }
    return 0;
}

void iadd(Num *self, Num *other)
{
    const num_class *cls = num_class_of(self);
    if (cls != NULL) {
        cls->iadd(self, other);
    }
}

Num *add(Num *self, Num *other)
{
    const num_class *cls = num_class_of(self);
    if (cls != NULL) {
        return cls->add(self, other);
    }
    return NULL;
}
