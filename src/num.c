#include <stdio.h>

#include <masc/num.h>


static void _vinit(Num *self, va_list va)
{
    object_init(self, NumCls);
}

int num_cmp(const Num *self, const Num *other)
{
    return 0;
}

size_t num_to_cstr(Num *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s at %p WIP!>", name_of(self), self);
}

int num_to_int(Num *self)
{
    return 0;
}

double num_to_double(Num *self)
{
    return 0;
}

static void _iadd_num(Num *self, Num *other)
{
}

static Num *_add_num(Num *self, Num *other)
{
    return new_copy(other);
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static num_class _NumCls = {
    .name = "Num",
    .size = sizeof(Num),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = object_init_copy,
    .destroy = object_destroy,
    .cmp = (cmp_cb)num_cmp,
    .repr = (repr_cb)num_to_cstr,
    .to_cstr = (to_cstr_cb)num_to_cstr,
    .to_int = (to_int_cb)num_to_int,
    .to_double = (to_double_cb)num_to_double,
    .iadd = (iadd_cb)_iadd_num,
    .add = (add_cb)_add_num,
};

const num_class *NumCls = &_NumCls;
