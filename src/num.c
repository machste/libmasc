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

Num *num_new_cstr(const char *cstr, bool strict)
{
    Num *self = malloc(sizeof(Num));
    num_init_cstr(self, cstr, strict);
    return self;
}

void num_init_cstr(Num *self, const char *cstr, bool strict)
{
    object_init(&self->obj, NumCls);
    num_set_cstr(self, cstr, strict);
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

bool num_set_cstr(Num *self, const char *cstr, bool strict)
{
    char *endptr;
    double value = strtod(cstr, &endptr);
    if ((!strict && endptr != cstr) || (strict && *endptr == '\0')) {
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

int num_cmp(const Num *self, const Num *other)
{
    if (self->val > other->val) {
        return 1;
    } else if (self->val < other->val) {
        return -1;
    } else {
        return 0;
    }
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
    .len = (len_cb)object_len,
    .cmp = (cmp_cb)num_cmp,
    .repr = (repr_cb)num_to_cstr,
    .to_cstr = (to_cstr_cb)num_to_cstr,
    .iter_init = NULL,
};

const Class *NumCls = &_NumCls;
