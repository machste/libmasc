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

size_t format(char *cstr, size_t size, const char *fmt, ...);
size_t vformat(char *cstr, size_t size, const char *fmt, va_list va);

void hexdump(const void *data, size_t size);

#endif /* _MASC_PRINT_H_ */
