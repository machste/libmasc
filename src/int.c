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
    object_init(&self->obj, IntCls);
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
    object_init(&self->obj, IntCls);
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
    if ((!strict && endptr != cstr) || (strict && *endptr == '\0')) {
        self->val = value;
        return true;
    }
    return false;
}

long int_iadd(Int *self, Int *other)
{
    return self->val += other->val;
}

Int *int_add(Int *self, Int *other)
{
    return int_new(self->val + other->val);
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


static Class _IntCls = {
    .name = "Int",
    .size = sizeof(Int),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .len = (len_cb)object_len,
    .cmp = (cmp_cb)int_cmp,
    .repr = (repr_cb)int_to_cstr,
    .to_cstr = (to_cstr_cb)int_to_cstr,
    .iter_init = NULL,
};

const Class *IntCls = &_IntCls;
