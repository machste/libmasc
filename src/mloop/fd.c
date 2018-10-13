#include <stdio.h>

#include "fd.h"


static void _vinit(MlFd *self, va_list va)
{
    object_init(self, MlFdCls);
    self->fd = va_arg(va, int);
    self->flags = va_arg(va, ml_fd_flag_t);
    self->cb = va_arg(va, ml_fd_cb);
    self->arg = va_arg(va, void *);
}


static class _MlFdCls = {
    .name = "MlFd",
    .size = sizeof(MlFd),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .len = (len_cb)object_len,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)object_to_cstr,
    .to_cstr = (to_cstr_cb)object_to_cstr,
    .iter_init = NULL,
};

const class *MlFdCls = &_MlFdCls;
