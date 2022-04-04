#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <masc/file.h>
#include <masc/path.h>
#include <masc/iter.h>
#include <masc/print.h>


#define FILE_READLINE_BUFFER 128


File *file_new(const char *path, const char *mode)
{
    File *self = malloc(sizeof(File));
    file_init(self, path, mode);
    return self;
}

void file_init(File *self, const char *path, const char *mode)
{
    object_init(self, FileCls);
    self->path = strdup(path);
    self->mode = strdup(mode);
    if (path_is_dir(path)) {
        self->file = NULL;
        self->errnum = EISDIR;
    } else {
        self->file = fopen(path, mode);
        if (self->file == NULL) {
            self->errnum = errno;
        } else {
            self->errnum = 0;
        }
    }
}

static void _vinit(File *self, va_list va)
{
    char *path = va_arg(va, char *);
    char *mode = va_arg(va, char *);
    file_init(self, path, mode);
}

void file_destroy(File *self)
{
    free(self->path);
    free(self->mode);
    file_close(self);
}

void file_delete(File *self)
{
    file_destroy(self);
    free(self);
}

const char *file_path(File *self)
{
    return self->path;
}

const char *file_basename(File *self)
{
    return path_basename(self->path);
}

int file_get_fd(File *self)
{
    int fd;
    if (self->file != NULL) {
        fd = fileno(self->file);
        if (fd < 0) {
            self->errnum = errno;
        }
    } else {
        self->errnum = EBADF;
        fd = -1;
    }
    return fd;
}

char *file_err_msg(File *self)
{
    return strerror(self->errnum);
}

size_t file_size(File *self)
{
    struct stat file_stats;
    int fd = file_get_fd(self);
    if (fd >= 0 && fstat(fd, &file_stats) == 0) {
        return file_stats.st_size;
    }
    return 0;
}

ssize_t file_read(File *self, void *data, size_t size)
{
    if (self->file == NULL) {
        return -1;
    }
    return fread(data, 1, size, self->file);
}

Str *file_readstr(File *self, long len) {
    if (self->file == NULL) {
        return NULL;
    }
    if (len < 0) {
        len = file_size(self) - ftell(self->file);
    }
    Str *s = str_new_ncopy(NULL, len);
    size_t read_len = fread(s->cstr, 1, len, self->file);
    if (read_len < len) {
        s->size = read_len + 1;
        s->cstr = realloc(s->cstr, s->size);
    }
    s->cstr[read_len] = '\0';
    return s;
}

Str *file_readline(File *self)
{
    if (self->file == NULL) {
        return NULL;
    }
    Str *line = NULL;
    char buf[FILE_READLINE_BUFFER];
    while(true) {
        int i, c;
        // Read bytes into buffer
        for (i = 0; i < sizeof(buf) - 1; i++) {
            c = fgetc(self->file);
            if (c == EOF) {
                break;
            } else if (c == '\n') {
                buf[i++] = c;
                break;
            }
            buf[i] = c;
        }
        buf[i] = '\0';
        // Check if EOF was the first read.
        if (i == 0 && line == NULL) {
            break;
        } else {
            if (line == NULL) {
                line = str_new_cstr(buf);
            } else {
                str_append(line, buf);
            }
            // Check if line is complete
            if (i < sizeof(buf) - 1 || buf[i - 1] == '\n') {
                break;
            }
        }
    }
    return line;
}

ssize_t file_write(File *self, const void *data, size_t size)
{
    if (self->file == NULL) {
        return -1;
    }
    return fwrite(data, 1, size, self->file);
}

void file_rewind(File *self)
{
    if (self->file != NULL) {
        rewind(self->file);
    }
}

bool file_reopen(File *self, const char *mode)
{
    if (self->file != NULL) {
        fclose(self->file);
    }
    self->file = fopen(self->path, mode);
    if (self->file == NULL) {
        self->errnum = errno;
        return false;
    } else {
        self->errnum = 0;
        return true;
    }
}

int file_close(File *self)
{
    int ret = -1;
    if (self->file != NULL) {
        ret = fclose(self->file);
        if (ret != 0) {
            self->errnum = errno;
        }
        self->file = NULL;
    }
    return ret;
}

int file_cmp(const File *self, const File *other)
{
    return strcmp(self->path, other->path);
}

size_t file_to_cstr(File *self, char *cstr, size_t size)
{
    char *state;
    if (self->file != NULL) {
        state = "open";
    } else if (self->errnum != 0) {
        state = strerror(self->errnum);
    } else {
        state = "closed";
    } 
    return snprintf(cstr, size, "<%s '%s', mode '%s', %s at %p>",
            name_of(self), self->path, self->mode, state, self);
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static io_class _FileCls = {
    .name = "File",
    .size = sizeof(File),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)file_destroy,
    .cmp = (cmp_cb)file_cmp,
    .repr = (repr_cb)file_to_cstr,
    .to_cstr = (to_cstr_cb)file_to_cstr,
    // Io Class
    .get_fd = (get_fd_cb)file_get_fd,
    .__read__ = (read_cb)file_read,
    .readstr = (readstr_cb)file_readstr,
    .readline = (readline_cb)file_readline,
    .__write__ = (write_cb)file_write,
    .__close__ = (close_cb)file_close,
};

const io_class *FileCls = &_FileCls;
