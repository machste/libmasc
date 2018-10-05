#ifndef _MASC_PRINT_H_
#define _MASC_PRINT_H_

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>


void put(const void *obj);
void put_repr(const void *obj);

size_t print(const char *fmt, ...);
size_t fprint(FILE *stream, const char *fmt, ...);
size_t vfprint(FILE *stream, const char *fmt, va_list va);
size_t dprint(int fd, const char *fmt, ...);
size_t vdprint(int fd, const char *fmt, va_list va);

size_t format(char *cstr, size_t size, const char *fmt, ...);
size_t vformat(char *cstr, size_t size, const char *fmt, va_list va);

size_t pretty_cstr(const void *obj, char *cstr, size_t size);

size_t pretty_print(const void *obj);


#endif /* _MASC_PRINT_H_ */
