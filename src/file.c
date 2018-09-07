#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <masc/file.h>


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

bool file_is_open(File *self)
{
    return self->file != NULL;
}

size_t file_size(File *self)
{
    struct stat file_stats;
    if (self->file != NULL) {
        int fd = fileno(self->file);
        if (fd == 0 && fstat(fd, &file_stats) == 0) {
            return file_stats.st_size;
        }
    }
    return 0;
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
