#ifndef _MASC_IO_H_
#define _MASC_IO_H_

#include <masc/io_class.h>


typedef struct {
    IoBase;
    int fd;
} Io;


extern const io_class *IoCls;


void io_init(Io *self, int fd);

void io_init_copy(Io *self, const Io *other);

void io_destroy(Io *self);

int io_get_fd(Io *self);

bool io_is_open(Io *self);

bool io_is_readable(Io *self);
bool io_is_writable(Io *self);

ssize_t io_read(Io *self, void *data, size_t size);
Str *io_readstr(IoBase *self, ssize_t len);
Str *io_readline(IoBase *self);

ssize_t io_write(Io *self, const void *data, size_t size);

int io_close(Io *self);

int io_cmp(const Io *self, const Io *other);

size_t io_to_cstr(Io *self, char *cstr, size_t size);


#endif /* _MASC_IO_H_ */
