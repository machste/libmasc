#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <masc/io_class.h>
#include <masc/io.h>
#include <masc/print.h>


const io_class *io_class_of(const IoBase *self)
{
    if (self != NULL) {
        return (io_class *)self->cls;
    } else {
        return NULL;
    }
}

int get_fd(IoBase *self)
{
    if (io_class_of(self) == NULL) {
        return -1;
    }
    return io_class_of(self)->get_fd(self);
}

bool is_open(IoBase *self)
{
    return fcntl(get_fd(self), F_GETFD) != -1;
}

bool is_readable(IoBase *self)
{
    int flags = fcntl(get_fd(self), F_GETFL, 0);
    return (flags != -1) && !(flags & O_WRONLY);
}

bool is_writable(IoBase *self)
{
    int flags = fcntl(get_fd(self), F_GETFL, 0);
    if (flags != -1) {
        flags &= O_ACCMODE;
        return flags == O_WRONLY || flags == O_RDWR;
    }
    return false;
}

bool is_blocking(IoBase *self)
{
    int flags = fcntl(get_fd(self), F_GETFL, 0);
    if (flags != -1) {
        return !(flags & O_NONBLOCK);
    }
    return false;
}

bool set_blocking(IoBase *self, bool blocking)
{
    int fd = get_fd(self);
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags != -1) {
        if (blocking) {
            flags &= ~O_NONBLOCK;
        } else {
            flags |= O_NONBLOCK;
        }
        return fcntl(fd, F_SETFL, flags) == 0;
    }
    return false;
}

ssize_t __read__(IoBase *self, void *data, size_t size)
{
    if (io_class_of(self) == NULL) {
        errno = EINVAL;
        return -1;
    }
    return io_class_of(self)->__read__(self, data, size);
}

Str *readstr(IoBase *self, ssize_t len)
{
    if (io_class_of(self) == NULL) {
        errno = EINVAL;
        return NULL;
    }
    return io_class_of(self)->readstr(self, len);
}

Str *readline(IoBase *self)
{
    if (io_class_of(self) == NULL) {
        errno = EINVAL;
        return NULL;
    }
    return io_class_of(self)->readline(self);
}

List *readlines(IoBase *self)
{
    List *l = new(List);
    Str *line;
    for (int i = 0; (line = readline(self)) != NULL; i++) {
        list_append(l, line);
    }
    return l;
}

ssize_t __write__(IoBase *self, const void *data, size_t size)
{
    if (io_class_of(self) == NULL) {
        errno = EINVAL;
        return -1;
    }
    return io_class_of(self)->__write__(self, data, size);
}

ssize_t writestr(IoBase *self, const char *cstr)
{
    return __write__(self, cstr, strlen(cstr));
}

ssize_t writefmt(IoBase *self, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    ssize_t len = writevfmt(self, fmt, va);
    va_end(va);
    return len;
}

ssize_t writevfmt(IoBase *self, const char *fmt, va_list va)
{
    va_list va2;
    // Make a copy of va to use it twice
    va_copy(va2, va);
    size_t cstr_len = vformat(NULL, 0, fmt, va);
    char *cstr = malloc(cstr_len + 1);
    vformat(cstr, cstr_len + 1, fmt, va2);
    ssize_t len = __write__(self, cstr, cstr_len);
    free(cstr);
    va_end(va2);
    return len;
}

ssize_t writeobj(IoBase *self, Object *obj)
{
    if (isinstance(obj, Str)) {
        return __write__(self, ((Str *)obj)->cstr, ((Str *)obj)->size - 1);
    } else {
        Str *s = new(Str, "%O\n", obj);
        ssize_t len = __write__(self, s->cstr, s->size - 1);
        delete(s);
        return len;
    }
}

ssize_t writelines(IoBase *self, Iterable *iterable)
{
    ssize_t len = 0;
    Iter *itr = new(Iter, iterable);
    for (Object *obj = next(itr); obj != NULL; obj = next(itr)) {
        ssize_t line_len = writeobj(self, obj);
        if (line_len >= 0) {
            len += line_len;
        } else {
            len = -1;
            break;
        }
    }
    delete(itr);
    return len;
}

int __close__(IoBase *self)
{
    if (io_class_of(self) == NULL) {
        errno = EINVAL;
        return -1;
    }
    return io_class_of(self)->__close__(self);
}
