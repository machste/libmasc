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
    object_init(&self->obj, FileCls);
    self->path = strdup(path);
    self->mode = strdup(mode);
    // TODO: Check if path is a directory!
    self->file = fopen(path, mode);
    if (self->file == NULL) {
        self->errnum = errno;
    } else {
        self->errnum = 0;
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

bool file_is_open(File *self)
{
    return self->file != NULL;
}

char *file_err_msg(File *self)
{
    return strerror(self->errnum);
}

size_t file_size(File *self)
{
    struct stat file_stats;
    if (self->file != NULL) {
        int fd = fileno(self->file);
        if (fd >= 0 && fstat(fd, &file_stats) == 0) {
            return file_stats.st_size;
        }
    }
    return 0;
}

Str *file_read(File *self, long len) {
    if (self->file == NULL) {
        return NULL;
    }
    if (len < 0) {
        len = file_size(self) - ftell(self->file);
    }
    Str *s = str_new_ncopy(NULL, len);
    long read_len = fread(s->cstr, 1, len, self->file);
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

List *file_readlines(File *self)
{
    List *l = new(List);
    Str *line;
    for (int i = 0; (line = file_readline(self)) != NULL; i++) {
        list_append(l, line);
    }
    return l;
}

long file_write(File *self, const char *cstr)
{
    if (self->file == NULL) {
        return -1;
    }
    return fwrite(cstr, 1, strlen(cstr), self->file);
}

long file_write_fmt(File *self, const char *fmt, ...)
{
    if (self->file == NULL) {
        return -1;
    }
    va_list va;
    va_start(va, fmt);
    long len = vfprint(self->file, fmt, va);
    va_end(va);
    return len;
}

long file_put(File *self, void *obj)
{
    Str *s = new(Str, "%O\n", obj);
    long len = file_write(self, str_cstr(s));
    delete(s);
    return len;
}

long file_writelines(File *self, void *iterable)
{
    long len = 0;
    Iter *itr = new(Iter, iterable);
    for (void *obj = next(itr); obj != NULL; obj = next(itr)) {
        long line_len;
        if (isinstance(obj, Str)) {
            line_len = file_write(self, ((Str *)obj)->cstr);
        } else {
            line_len = file_put(self, obj);
        }
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

size_t file_to_cstr(File *self, char *cstr, size_t size)
{
    char *state;
    if (file_is_open(self)) {
        state = "open";
    } else if (self->errnum != 0) {
        state = strerror(self->errnum);
    } else {
        state = "closed";
    } 
    return snprintf(cstr, size, "<%s '%s', mode '%s', %s at %p>",
            name_of(self), self->path, self->mode, state, self);
}


static Class _FileCls = {
    .name = "File",
    .size = sizeof(File),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)file_destroy,
    .repr = (repr_cb)file_to_cstr,
    .to_cstr = (to_cstr_cb)file_to_cstr,
    .iter_init = NULL,
};

const void *FileCls = &_FileCls;
