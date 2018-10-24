#include "fd.h"
#include "fdreader.h"


#define FD_READER_BUFFER_INCREASE 512


static void _reader_data_cb(MlFdReader *self, int fd, ml_fd_flag_t events,
        void *arg)
{
    if (events & ML_FD_READ) {
        // Read data
        size_t buf_size = 0;
        size_t pos = 0;
        void *buf = NULL;
        while(true) {
            if (buf_size <= pos) {
                buf_size += FD_READER_BUFFER_INCREASE;
                buf = realloc(buf, buf_size);
            }
            ssize_t len = read(self->fd, buf + pos, buf_size - pos);
            if (len > 0) {
                pos += len;
            } else {
                break;
            }
        }
        if(pos > 0 && self->data_cb != NULL) {
            self->data_cb(self, buf, pos, arg);
        }
        free(buf);
    }
    if (events & ML_FD_EOF) {
        if(self->eof_cb != NULL) {
            self->eof_cb(self, arg);
        }
    }
}

static void _reader_vinit(MlFdReader *self, va_list va)
{
    int fd = va_arg(va, int);
    self->data_cb = va_arg(va, ml_fd_data_cb);
    self->eof_cb = va_arg(va, ml_fd_eof_cb);
    void *arg = va_arg(va, void *);
    __init__(MlFdCls, self, fd, ML_FD_READ, _reader_data_cb, arg);
}


static class _MlFdReaderCls = {
    .name = "MlFdReader",
    .size = sizeof(MlFdReader),
    .vinit = (vinit_cb)_reader_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)object_to_cstr,
    .to_cstr = (to_cstr_cb)object_to_cstr,
};

const class *MlFdReaderCls = &_MlFdReaderCls;
