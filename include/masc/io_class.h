#ifndef _MASC_IO_CLASS_H_
#define _MASC_IO_CLASS_H_

#include <unistd.h>
#include <stdbool.h>

#include <masc/object.h>
#include <masc/str.h>
#include <masc/list.h>
#include <masc/iter.h>


#define read(self, data, size) \
        _Generic(self, int: read, default: __read__)(self, data, size)

#define write(self, data, size) \
        _Generic(self, int: write, default: __write__)(self, data, size)

#define close(self) _Generic((self), int: close, default: __close__)(self)


typedef struct IoBase {
    Object;
} IoBase;

typedef int (*get_fd_cb)(IoBase *self);
typedef ssize_t (*read_cb)(IoBase *self, void *data, size_t size);
typedef Str *(*readstr_cb)(IoBase *self, ssize_t len);
typedef Str *(*readline_cb)(IoBase *self);
typedef ssize_t (*write_cb)(IoBase *self, const void *data, size_t size);
typedef int (*close_cb)(IoBase *self);

typedef struct {
    class;
    get_fd_cb get_fd;
    read_cb __read__;
    readstr_cb readstr;
    readline_cb readline;
    write_cb __write__;
    close_cb __close__;
} io_class;

const io_class *io_class_of(const IoBase *self);

int get_fd(IoBase *self);

bool is_open(IoBase *self);

bool is_readable(IoBase *self);
bool is_writable(IoBase *self);

bool is_blocking(IoBase *self);
bool set_blocking(IoBase *self, bool blocking);

ssize_t __read__(IoBase *self, void *data, size_t size);
Str *readstr(IoBase *self, ssize_t len);
Str *readline(IoBase *self);
List *readlines(IoBase *self);

ssize_t __write__(IoBase *self, const void *data, size_t size);
ssize_t writestr(IoBase *self, const char *cstr);
ssize_t writefmt(IoBase *self, const char *fmt, ...);
ssize_t writevfmt(IoBase *self, const char *fmt, va_list va);
ssize_t writeobj(IoBase *self, Object *obj);
ssize_t writelines(IoBase *self, Iterable *iterable);

int __close__(IoBase *self);

#endif /* _MASC_IO_CLASS_H_ */
