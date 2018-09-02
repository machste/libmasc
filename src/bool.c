#include <masc/bool.h>
#include <masc/cstr.h>


Bool *bool_new(bool b)
{
    Bool *self = malloc(sizeof(Bool));
    bool_init(self, b);
    return self;
}

void bool_init(Bool *self, bool b)
{
    object_init(&self->obj, BoolCls);
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

size_t bool_to_cstr(Bool *self, char *cstr, size_t size)
{
    if (self->b) {
        return cstr_ncopy(cstr, "true", size);
    } else {
        return cstr_ncopy(cstr, "false", size);
    }
}


static Class _BoolCls = {
    .name = "Bool",
    .size = sizeof(Bool),
    .vinit = (vinit_cb)bool_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .repr = (repr_cb)bool_to_cstr,
    .to_cstr = (to_cstr_cb)bool_to_cstr,
};

const void *BoolCls = &_BoolCls;
