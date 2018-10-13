#include <stdio.h>

#include "timer.h"


static void _vinit(MlTimer *self, va_list va)
{
    object_init(self, MlTimerCls);
    self->pending = false;
    self->msec = 0;
    self->time = 0;
    self->cb = va_arg(va, ml_timer_cb);
    self->arg = va_arg(va, void *);
}

int mloop_timer_remaining(MlTimer *self)
{
    if (!self->pending) {
        return -1;
    }
    return mloop_time() - self->time;
}

int mloop_timer_msec(MlTimer *self)
{
    return self->msec;
}

int ml_timer_cmp(MlTimer *self, MlTimer *other)
{
    return self->time - other->time;
}

static size_t _to_cstr(MlTimer *self, char *cstr, size_t size)
{
    if (self->pending) {
        return snprintf(cstr, size, "<%s msec: %i, pending at %p>",
                name_of(self), self->msec, self);
    }
    return snprintf(cstr, size, "<%s msec: %i, not pending at %p>",
            name_of(self), self->msec, self);
}


static class _MlTimerCls = {
    .name = "MlTimer",
    .size = sizeof(MlTimer),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .len = (len_cb)object_len,
    .cmp = (cmp_cb)ml_timer_cmp,
    .repr = (repr_cb)_to_cstr,
    .to_cstr = (to_cstr_cb)_to_cstr,
    .iter_init = NULL,
};

const class *MlTimerCls = &_MlTimerCls;
