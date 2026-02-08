#include <stdio.h>

#include <masc/mloop.h>


MlTimer *ml_timer_new(ml_timer_cb cb, void *arg)
{
    MlTimer *self = malloc(sizeof(MlTimer));
    ml_timer_init(self, cb, arg);
    return self;
}

void ml_timer_init(MlTimer *self, ml_timer_cb cb, void *arg)
{
    object_init(self, MlTimerCls);
    self->pending = false;
    self->msec = 0;
    self->time = 0;
    self->cb = cb;
    self->arg = arg;
}

static void _vinit(MlTimer *self, va_list va)
{
    ml_timer_cb cb = va_arg(va, ml_timer_cb);
    void *arg = va_arg(va, void *);
    ml_timer_init(self, cb, arg);
}

void ml_timer_destroy(MlTimer *self)
{
    mloop_timer_cancle(self);
    object_destroy(self);
}

void ml_timer_delete(MlTimer *self)
{
    ml_timer_destroy(self);
    free(self);
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

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static class _MlTimerCls = {
    .name = "MlTimer",
    .size = sizeof(MlTimer),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)ml_timer_destroy,
    .cmp = (cmp_cb)ml_timer_cmp,
    .repr = (repr_cb)_to_cstr,
    .to_cstr = (to_cstr_cb)_to_cstr,
};

const class *MlTimerCls = &_MlTimerCls;
