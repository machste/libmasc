#ifndef _MASC_BOOL_H_
#define _MASC_BOOL_H_

#include <stdbool.h>

#include <masc/object.h>


typedef struct {
    Object obj;
    bool b;
} Bool;

extern const char *const bool_true_cstr;
extern const char *const bool_false_cstr;
extern const void *BoolCls;


Bool *bool_new(bool b);
void bool_init(Bool *self, bool b);
void bool_vinit(Bool *self, va_list va);

void bool_delete(Bool *self);

bool bool_get(Bool *self);
void bool_set(Bool *self, bool b);
bool bool_toggle(Bool *self);

int bool_cmp(const Bool *self, const Bool *other);

size_t bool_to_cstr(Bool *self, char *cstr, size_t size);

#endif /* _MASC_BOOL_H_ */
