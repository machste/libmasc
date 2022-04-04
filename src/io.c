#include <stdio.h>

#include <masc/io.h>


void io_init(Io *self, int fd)
{
    object_init(self, IoCls);
    self->fd = fd;
}

static void _vinit(Io *self, va_list va)
{
    int fd = va_arg(va, int);
    io_init(self, fd);
}

void io_init_copy(Io *self, const Io *other)
{
    object_init(self, IoCls);
    self->fd = dup(other->fd);
}

void io_destroy(Io *self)
{
    close(self->fd);
}

int io_get_fd(Io *self)
{
    return self->fd;
}

ssize_t io_read(Io *self, void *data, size_t size)
{
    return read(self->fd, data, size);
}

Str *io_readstr(IoBase *self, ssize_t len)
{
    return NULL;
}

Str *io_readline(IoBase *self)
{
    return NULL;
}

ssize_t io_write(Io *self, const void *data, size_t size)
{
    return write(self->fd, data, size);
}

int io_close(Io *self)
{
    return close(self->fd);
}

int io_cmp(const Io *self, const Io *other)
{
    if (self->fd > other->fd) {
        return 1;
    } else if (self->fd < other->fd) {
        return -1;
    } else {
        return 0;
    }
}

size_t io_to_cstr(Io *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s fd: %i at %p>",
            name_of(self), self->fd, self);
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


io_class _IoCls = {
    .name = "Io",
    .size = sizeof(Io),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)io_init_copy,
    .destroy = (destroy_cb)io_destroy,
    .cmp = (cmp_cb)io_cmp,
    .repr = (repr_cb)io_to_cstr,
    .to_cstr = (to_cstr_cb)io_to_cstr,
    // Io Class
    .get_fd = (get_fd_cb)io_get_fd,
    .__read__ = (read_cb)io_read,
    .readstr = (readstr_cb)io_readstr,
    .readline = (readline_cb)io_readline,
    .__write__ = (write_cb)io_write,
    .__close__ = (close_cb)io_close,
};

const io_class *IoCls = &_IoCls;
