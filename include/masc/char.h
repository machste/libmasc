#ifndef _MASC_CHAR_H_
#define _MASC_CHAR_H_

#include <stdbool.h>

#include <masc/object.h>


typedef struct {
    Object obj;
    char c;
} Char;


extern const void *CharCls;


Char *char_new(char c);
void char_init(Char *self, char c);
void char_vinit(Char *self, va_list va);

void char_delete(Char *self);

char char_get(Char *self);
void char_set(Char *self, char c);

bool char_is_alpha(Char *self);

int char_cmp(const Char *self, const Char *other);

size_t char_repr(Char *self, char *cstr, size_t size);
size_t char_to_cstr(Char *self, char *cstr, size_t size);

#endif /* _MASC_CHAR_H_ */
