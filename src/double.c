#include <stdlib.h>
#include <stdio.h>

#include <masc/double.h>


Double *double_new(double val)
{
    Double *self = malloc(sizeof(Double));
    double_init(self, val);
    return self;
}

void double_init(Double *self, double val)
{
    object_init(self, DoubleCls);
    self->val = val;
}

static void _vinit(Double *self, va_list va)
{
    double val = va_arg(va, double);
    double_init(self, val);
}

Double *double_new_cstr(const char *cstr, bool strict)
{
    Double *self = malloc(sizeof(Double));
    double_init_cstr(self, cstr, strict);
    return self;
}

void double_init_cstr(Double *self, const char *cstr, bool strict)
{
    object_init(self, DoubleCls);
    double_set_cstr(self, cstr, strict);
}

void double_delete(Double *self)
{
    free(self);
}

double double_get(Double *self)
{
    return self->val;
}

void double_set(Double *self, double value)
{
    self->val = value;
}

bool double_set_cstr(Double *self, const char *cstr, bool strict)
{
    char *endptr;
    double value = strtod(cstr, &endptr);
    if (endptr != cstr && (!strict || (strict && *endptr == '\0'))) {
        self->val = value;
        return true;
    }
    return false;
}

double double_iadd(Double *self, double other)
{
    return self->val += other;
}

static void _iadd_num(Double *self, Num *other)
{
    double_iadd(self, to_double(other));
}

Double *double_add(Double *self, double other)
{
    return double_new(self->val + other);
}

static Double *_add_num(Double *self, Num *other)
{
    return double_add(self, to_double(other));
}

int double_cmp(const Double *self, const Double *other)
{
    if (self->val > other->val) {
        return 1;
    } else if (self->val < other->val) {
        return -1;
    } else {
        return 0;
    }
}

size_t double_to_cstr(Double *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "%g", self->val);
}

long double_to_int(Double *self)
{
    return (long)self->val;
}

static num_class _DoubleCls = {
    .name = "Double",
    .size = sizeof(Double),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .cmp = (cmp_cb)double_cmp,
    .repr = (repr_cb)double_to_cstr,
    .to_cstr = (to_cstr_cb)double_to_cstr,
    .to_int = (to_int_cb)double_to_int,
    .to_double = (to_double_cb)double_get,
    .iadd = (iadd_cb)_iadd_num,
    .add = (add_cb)_add_num,
};

const num_class *DoubleCls = &_DoubleCls;