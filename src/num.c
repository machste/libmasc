#include <stdlib.h>
#include <stdio.h>

#include <masc/num.h>


Num *num_new(double val)
{
    Num *self = malloc(sizeof(Num));
    num_init(self, val);
    return self;
}

void num_init(Num *self, double val)
{
    object_init(&self->obj, NumCls);
    self->val = val;
}

static void _vinit(Num *self, va_list va)
{
    double val = va_arg(va, double);
    num_init(self, val);
}

Num *num_new_cstr(const char *cstr)
{
    Num *self = malloc(sizeof(Num));
    num_init_cstr(self, cstr);
    return self;
}

void num_init_cstr(Num *self, const char *cstr)
{
    object_init(&self->obj, NumCls);
    num_set_cstr(self, cstr);
}

void num_delete(Num *self)
{
    free(self);
}

double num_get(Num *self)
{
    return self->val;
}

void num_set(Num *self, double value)
{
    self->val = value;
}

bool num_set_cstr(Num *self, const char *cstr)
{
    char *endptr;
    double value = strtod(cstr, &endptr);
    if (endptr != cstr) {
        self->val = value;
        return true;
    }
    return false;
}

double num_iadd(Num *self, Num *other)
{
    return self->val += other->val;
}

Num *num_add(Num *self, Num *other)
{
    return num_new(self->val + other->val);
}

size_t num_to_cstr(Num *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "%g", self->val);
}


static Class _NumCls = {
    .name = "Num",
    .size = sizeof(Num),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .repr = (repr_cb)num_to_cstr,
    .to_cstr = (to_cstr_cb)num_to_cstr,
    .iter_init = NULL,
};

const Class *NumCls = &_NumCls;
