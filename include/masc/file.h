#ifndef _MASC_FILE_H_
#define _MASC_FILE_H_

#include <stdio.h>
#include <stdbool.h>

#include <masc/object.h>
#include <masc/str.h>
#include <masc/list.h>


typedef struct {
    Object obj;
    char *path;
    char *mode;
    FILE *file;
    int errnum;
} File;


extern const void *FileCls;


File *file_new(const char *path, const char *mode);
void file_init(File *self, const char *path, const char *mode);

void file_destroy(File *self);
void file_delete(File *self);

const char *file_path(File *self);
const char *file_basename(File *self);

bool file_is_open(File *self);
char *file_err_msg(File *self);

size_t file_size(File *self);

Str *file_read(File *self, long len);
Str *file_readline(File *self);
List *file_readlines(File *self);

long file_write(File *self, const char *cstr);
long file_write_fmt(File *self, const char *fmt, ...);
long file_put(File *self, void *obj);
long file_writelines(File *self, void *iterable);

void file_rewind(File *self);

bool file_reopen(File *self, const char *mode);
int file_close(File *self);

size_t file_to_cstr(File *self, char *cstr, size_t size);

#endif /* _MASC_FILE_H_ */
