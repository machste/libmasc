#ifndef _MASC_PRINT_H_
#define _MASC_PRINT_H_

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

void put(void *self);

size_t print(char const *fmt, ...);
size_t fprint(FILE *stream, char const *fmt, ...);
size_t vfprint(FILE *stream, char const *fmt, va_list va);

size_t format(char *cstr, size_t size, char const *fmt, ...);
size_t vformat(char *cstr, size_t size, char const *fmt, va_list va);

void hexdump(const void *data, size_t size);

#endif /* _MASC_PRINT_H_ */
