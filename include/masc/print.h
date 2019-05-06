#ifndef _MASC_PRINT_H_
#define _MASC_PRINT_H_

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>


int put(const void *obj);
int put_repr(const void *obj);

ssize_t print(const char *fmt, ...);
ssize_t fprint(FILE *stream, const char *fmt, ...);
ssize_t vfprint(FILE *stream, const char *fmt, va_list va);
ssize_t dprint(int fd, const char *fmt, ...);
ssize_t vdprint(int fd, const char *fmt, va_list va);

size_t format(char *cstr, size_t size, const char *fmt, ...);
size_t vformat(char *cstr, size_t size, const char *fmt, va_list va);

size_t pretty_cstr(const void *obj, char *cstr, size_t size);

ssize_t pretty_print(const void *obj);


#endif /* _MASC_PRINT_H_ */
