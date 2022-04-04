#include <string.h>

#include "ioreader.h"
#include "iopkg.h"


static void _pkg_data_cb(MlIoPkg *self, void *_data, size_t _size, void *arg)
{
    char *data = _data;
    size_t size = _size;
    if (self->data != NULL) {
        // Append new data to existing
        data = self->data = realloc(self->data, self->size + _size);
        memcpy(self->data + self->size, _data, _size);
        size = self->size += _size;
    }
    // Search for complete packets which end with a sentinel
    size_t i = 0, pos = 0;
    for (i = 0; i < size; i++) {
        if (data[i] == self->sentinel) {
            if (self->pkg_cb != NULL) {
                self->pkg_cb(self, data + pos, i - pos + 1, arg);
            }
            // Set new start position and skip the sentinel
            pos = i + 1;
        }
    }
    // Copy remaining data to MlFdPkg object for the next round
    if (pos < i) {
        self->size = i - pos;
        self->data = realloc(self->data, self->size);
        memcpy(self->data, data + pos, self->size);
    } else {
        free(self->data);
        self->data = NULL;
        self->size = 0;
    }
}

static void _pkg_vinit(MlIoPkg *self, va_list va)
{
    IoBase *io = va_arg(va, IoBase *);
    self->sentinel = (char)va_arg(va, int);
    self->pkg_cb = va_arg(va, ml_io_pkg_cb);
    ml_io_eof_cb eof_cb = va_arg(va, ml_io_eof_cb);
    void *arg = va_arg(va, void *);
    __init__(MlIoReaderCls, self, io, _pkg_data_cb, eof_cb, arg);
    self->data = NULL;
    self->size = 0;
}

static void _pkg_destroy(MlIoPkg *self)
{
    if (self->data != NULL) {
        free(self->data);
    }
}

static void _init_class(class *cls)
{
    cls->super = MlIoReaderCls;
}


static class _MlIoPkgCls = {
    .name = "MlIoPkg",
    .size = sizeof(MlIoPkg),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_pkg_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)_pkg_destroy,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)object_to_cstr,
    .to_cstr = (to_cstr_cb)object_to_cstr,
};

const class *MlIoPkgCls = &_MlIoPkgCls;
