#include "io.h"


static void _io_vinit(MlIo *self, va_list va)
{
    object_init(self, MlIoCls);
    self->io = va_arg(va, IoBase *);
    self->flags = va_arg(va, ml_io_flag_t);
    self->cb = va_arg(va, ml_io_cb);
    self->arg = va_arg(va, void *);
}


static class _MlIoCls = {
    .name = "MlIo",
    .size = sizeof(MlIo),
    .vinit = (vinit_cb)_io_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)object_to_cstr,
    .to_cstr = (to_cstr_cb)object_to_cstr,
};

const class *MlIoCls = &_MlIoCls;
