#include <stdlib.h>
#include <stdio.h>

#include <masc/int.h>


Int *int_new(long val)
{
    Int *self = malloc(sizeof(Int));
    int_init(self, val);
    return self;
}

void int_init(Int *self, long val)
{
    object_init(self, IntCls);
    self->val = val;
}

static void _vinit(Int *self, va_list va)
{
    long val = va_arg(va, long);
    int_init(self, val);
}

Int *int_new_cstr(const char *cstr, bool strict)
{
    Int *self = malloc(sizeof(Int));
    int_init_cstr(self, cstr, strict);
    return self;
}

void int_init_cstr(Int *self, const char *cstr, bool strict)
{
    object_init(self, IntCls);
    int_set_cstr(self, cstr, strict);
}

void int_delete(Int *self)
{
    free(self);
}

long int_get(Int *self)
{
    return self->val;
}

void int_set(Int *self, long value)
{
    self->val = value;
}

bool int_set_cstr(Int *self, const char *cstr, bool strict)
{
    char *endptr;
    long value = strtol(cstr, &endptr, 0);
    if (endptr != cstr && (!strict || (strict && *endptr == '\0'))) {
        self->val = value;
        return true;
    }
    return false;
}


long int_iadd(Int *self, long other)
{
    return self->val += other;
}

static void _iadd_num(Int *self, Num *other)
{
    int_iadd(self, to_int(other));
}

Int *int_add(Int *self, long other)
{
    return int_new(self->val + other);
}

static Int *_add_num(Int *self, Num *other)
{
    return int_add(self, to_int(other));
}

bool int_in_range(Int *self, long start, long stop)
{
    return self->val >= start && self->val <= stop;
}

int int_cmp(const Int *self, const Int *other)
{
    if (self->val > other->val) {
        return 1;
    } else if (self->val < other->val) {
        return -1;
    } else {
        return 0;
    }
}

size_t int_to_cstr(Int *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "%li", self->val);
}

double int_to_double(Int *self)
{
    return (double)int_get(self);
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static num_class _IntCls = {
    .name = "Int",
    .size = sizeof(Int),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .cmp = (cmp_cb)int_cmp,
    .repr = (repr_cb)int_to_cstr,
    .to_cstr = (to_cstr_cb)int_to_cstr,
    .to_int = (to_int_cb)int_get,
    .to_double = (to_double_cb)int_to_double,
    .iadd = (iadd_cb)_iadd_num,
    .add = (add_cb)_add_num,
};

const num_class *IntCls = &_IntCls;
