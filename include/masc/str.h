#ifndef _MASC_STR_H_
#define _MASC_STR_H_

#include <masc/object.h>
#include <masc/iter.h>


typedef struct {
    Object obj;
    char *cstr;
    size_t size;
} Str;


extern const void *StrCls;


Str *str_new(const char *cstr);
void str_init(Str *self, const char *cstr);

Str *str_new_copy(Str *other);
void str_init_copy(Str *self, Str *other);

Str *str_new_len(size_t len);
void str_init_len(Str *self, size_t len);

Str *str_new_fmt(const char *fmt, ...);
void str_init_fmt(Str *self, const char *fmt, ...);
void str_init_vfmt(Str *self, const char *fmt, va_list va);
void str_vinit(Str *self, va_list va);

void str_destroy(Str *self);
void str_delete(Str *self);

size_t str_len(Str *self);

char *str_cstr(Str *self);
size_t str_repr(Str *self, char *cstr, size_t size);
size_t str_to_cstr(Str *self, char *cstr, size_t size);

Iter str_iter(Str *self);

void str_copy(Str *self, const char *cstr);

char *str_fmt(Str *self, const char *fmt, ...);

char *str_lower(Str *self);
char *str_upper(Str *self);
char *str_swapcase(Str *self);

char *str_lstrip(Str *self);
char *str_rstrip(Str *self);
char *str_strip(Str *self);

Str *to_str(void *self);

#endif /* _MASC_STR_H_ */