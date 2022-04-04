#include <masc/bool.h>
#include <masc/cstr.h>


const char *const bool_true_cstr = "true";
const char *const bool_false_cstr = "false";


Bool *bool_new(bool b)
{
    Bool *self = malloc(sizeof(Bool));
    bool_init(self, b);
    return self;
}

void bool_init(Bool *self, bool b)
{
    object_init(self, BoolCls);
    self->b = b;
}

void bool_vinit(Bool *self, va_list va)
{
    bool b = (bool)va_arg(va, int);
    bool_init(self, b);
}

void bool_delete(Bool *self)
{
    free(self);
}

bool bool_get(Bool *self)
{
    return self->b;
}

void bool_set(Bool *self, bool b)
{
    self->b = b;
}

bool bool_toggle(Bool *self)
{
    self->b = !self->b;
    return self->b;
}

int bool_cmp(const Bool *self, const Bool *other)
{
    if (self->b == other->b) {
        return 0;
    } else if (self->b) {
        return 1;
    } else {
        return -1;
    }
}

size_t bool_to_cstr(Bool *self, char *cstr, size_t size)
{
    if (self->b) {
        return cstr_ncopy(cstr, bool_true_cstr, size);
    } else {
        return cstr_ncopy(cstr, bool_false_cstr, size);
    }
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static class _BoolCls = {
    .name = "Bool",
    .size = sizeof(Bool),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)bool_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .cmp = (cmp_cb)bool_cmp,
    .repr = (repr_cb)bool_to_cstr,
    .to_cstr = (to_cstr_cb)bool_to_cstr,
};

const class *BoolCls = &_BoolCls;
