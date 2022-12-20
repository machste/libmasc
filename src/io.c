#include <stdio.h>

#include <masc/io.h>
#include <masc/math.h>


#define IO_READ_BUFFER 128


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

static Str *_read_until(Io *self, char key, ssize_t len)
{
    Str *str = NULL;
    size_t buf_len = (len >= 0) ? max(len, IO_READ_BUFFER) : IO_READ_BUFFER;
    char buf[buf_len];
    while(true) {
        int i;
        // Read bytes into buffer
        for (i = 0; i < buf_len - 1; i++) {
            char c;
            ssize_t r = read(self->fd, &c, 1);
            if (r <= 0) {
                break;
            } else if (c == key) {
                buf[i++] = c;
                break;
            }
            buf[i] = c;
        }
        buf[i] = '\0';
        // Check if an error occured on the first read
        if (i == 0 && str == NULL) {
            break;
        } else {
            if (str == NULL) {
                str = str_new_cstr(buf);
            } else {
                str_append(str, buf);
            }
            // Check if str is complete
            if (i < buf_len - 1 || buf[i - 1] == key) {
                break;
            }
        }
    }
    return str;
}

Str *io_readstr(Io *self, ssize_t len)
{
    return _read_until(self, '\0', len);
}

Str *io_readline(Io *self)
{
    return _read_until(self, '\n', -1);
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
