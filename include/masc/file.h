#ifndef _MASC_FILE_H_
#define _MASC_FILE_H_

#include <stdio.h>
#include <stdbool.h>

#include <masc/io.h>
#include <masc/str.h>
#include <masc/list.h>


typedef struct {
    IoBase;
    char *path;
    char *mode;
    FILE *file;
    int errnum;
} File;


extern const io_class *FileCls;


File *file_new(const char *path, const char *mode);
void file_init(File *self, const char *path, const char *mode);

void file_destroy(File *self);
void file_delete(File *self);

const char *file_path(File *self);
const char *file_basename(File *self);

char *file_err_msg(File *self);

size_t file_size(File *self);

ssize_t file_read(File *self, void *data, size_t size);
Str *file_readstr(File *self, long len);
Str *file_readline(File *self);

ssize_t file_write(File *self, const void *data, size_t size);

void file_rewind(File *self);

bool file_reopen(File *self, const char *mode);
int file_close(File *self);

int file_cmp(const File *self, const File *other);

size_t file_to_cstr(File *self, char *cstr, size_t size);

#endif /* _MASC_FILE_H_ */
