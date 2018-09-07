#ifndef _MASC_FILE_H_
#define _MASC_FILE_H_

#include <stdio.h>
#include <stdbool.h>

#include <masc/object.h>
#include <masc/str.h>


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

bool file_is_open(File *self);
size_t file_size(File *self);

Str *file_readline(File *self);

int file_close(File *self);

size_t file_to_cstr(File *self, char *cstr, size_t size);

#endif /* _MASC_FILE_H_ */
