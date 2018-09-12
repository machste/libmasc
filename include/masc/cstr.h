#ifndef _MASC_CSTR_H_
#define _MASC_CSTR_H_

#include <stdlib.h>

size_t cstr_putc(char *dest, const char c, size_t size);

size_t cstr_ncopy(char *dest, const char *src, size_t size);

size_t cstr_escape(char *dest, const char *src, size_t src_len, size_t size);
size_t cstr_unescape(char *dest, const char *src, size_t size);

size_t cstr_startswith(const char *cstr, const char *start);

#endif /* _MASC_CSTR_H_ */
