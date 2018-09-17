#include <stdio.h>
#include <ctype.h>

#include <masc/char.h>


Char *char_new(char c)
{
    Char *self = malloc(sizeof(Char));
    char_init(self, c);
    return self;
}

void char_init(Char *self, char c)
{
    object_init(&self->obj, CharCls);
    self->c = c;
}

void char_vinit(Char *self, va_list va)
{
    char c = (char)va_arg(va, int);
    char_init(self, c);
}

void char_delete(Char *self)
{
    free(self);
}

char char_get(Char *self)
{
    return self->c;
}

void char_set(Char *self, char c)
{
    self->c = c;
}

bool char_is_alpha(Char *self) {
    return (bool)isalpha(self->c);
}

int char_cmp(const Char *self, const Char *other)
{
    return self->c - other->c;
}

size_t char_repr(Char *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "\"%c\"", self->c);
}


size_t char_to_cstr(Char *self, char *cstr, size_t size)
{
    if (size > 1) {
        cstr[0] = self->c;
        cstr[1] = '\0';
    }
    return 1;
}


static Class _CharCls = {
    .name = "Char",
    .size = sizeof(Char),
    .vinit = (vinit_cb)char_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .len = (len_cb)object_len,
    .cmp = (cmp_cb)char_cmp,
    .repr = (repr_cb)char_repr,
    .to_cstr = (to_cstr_cb)char_to_cstr,
    .iter_init = NULL,
};

const void *CharCls = &_CharCls;
