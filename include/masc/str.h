#ifndef _MASC_STR_H_
#define _MASC_STR_H_

#include <stdbool.h>

#include <masc/object.h>
#include <masc/list.h>


typedef struct {
    Object obj;
    char *cstr;
    size_t size;
} Str;


extern const Class *StrCls;


Str *str_new(const char *fmt, ...);
void str_init(Str *self, const char *fmt, ...);
void str_vinit(Str *self, const char *fmt, va_list va);

Str *str_new_cstr(const char *cstr);
void str_init_cstr(Str *self, const char *cstr);

Str *str_new_copy(const Str *other);
void str_init_copy(Str *self, const Str *other);

Str *str_new_ncopy(const char* cstr, size_t len);
void str_init_ncopy(Str *self, const char* cstr, size_t len);

Str *str_new_slice(Str *other, size_t start, size_t end);
void str_init_slice(Str *self, Str *other, size_t start, size_t end);

void str_destroy(Str *self);
void str_delete(Str *self);

size_t str_len(Str *self);
bool str_is_empty(Str *self);

char *str_cstr(Str *self);
size_t str_repr(Str *self, char *cstr, size_t size);
size_t str_to_cstr(Str *self, char *cstr, size_t size);

char str_get_at(Str *self, size_t index);
void str_set_at(Str *self, size_t index, char c);
void str_delete_at(Str *self, size_t index);

Str *str_copy(Str *self, const char *cstr);
Str *str_append(Str *self, const char *cstr);
Str *str_append_fmt(Str *self, const char *fmt, ...);

Str *str_fmt(Str *self, const char *fmt, ...);

Str *str_lower(Str *self);
Str *str_upper(Str *self);
Str *str_swapcase(Str *self);

Str *str_reverse(Str *self);

Str *str_lstrip(Str *self);
Str *str_rstrip(Str *self);
Str *str_strip(Str *self);

Str *str_slice(Str *self, size_t start, size_t end);

List *str_split(Str *self, const char *sep, int maxsplit);
Str *str_join(void *iterable, const char *sep);

Str *to_str(const void *self);

#endif /* _MASC_STR_H_ */
