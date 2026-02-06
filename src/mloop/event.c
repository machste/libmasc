#include <stdio.h>

#include <masc/mloop.h>


MlEvent *ml_event_new(ml_event_cb cb, void *arg)
{
    MlEvent *self = malloc(sizeof(MlEvent));
    ml_event_init(self, cb, arg);
    return self;
}

void ml_event_init(MlEvent *self, ml_event_cb cb, void *arg)
{
    object_init(self, MlEventCls);
    self->fired = false;
    self->cb = cb;
    self->arg = arg;
}

static void _vinit(MlEvent *self, va_list va)
{
    ml_event_cb cb = va_arg(va, ml_event_cb);
    void *arg = va_arg(va, void *);
    ml_event_init(self, cb, arg);
}

void ml_event_destroy(MlEvent *self)
{
    mloop_event_remove(self);
    object_destroy(self);
}

void ml_event_delete(MlEvent *self)
{
    ml_event_destroy(self);
    free(self);
}

static size_t _to_cstr(MlEvent *self, char *cstr, size_t size)
{
    const char *fired_cstr = self->fired ? "" : "not ";
    return snprintf(cstr, size, "<%s %spending at %p>",
            name_of(self), fired_cstr, self);
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static class _MlEventCls = {
    .name = "MlEvent",
    .size = sizeof(MlEvent),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)ml_event_destroy,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)_to_cstr,
    .to_cstr = (to_cstr_cb)_to_cstr,
};

const class *MlEventCls = &_MlEventCls;
