#ifndef _MASC_CLASS_H_
#define _MASC_CLASS_H_

#include <stdlib.h>
#include <stdarg.h>

#define new(cls, ...) __new__(cls##Cls, ##__VA_ARGS__)
#define init(cls, ...) ({cls s; __init__(cls##Cls, &s, ##__VA_ARGS__);s;})


typedef void (*vinit_cb)(void *self, va_list va);
typedef void (*init_copy_cb)(void *self, const void *other);
typedef void (*destroy_cb)(void *self);

typedef size_t (*repr_cb)(const void *self, char *cstr, size_t size);
typedef size_t (*to_cstr_cb)(const void *self, char *cstr, size_t size);

typedef struct {
    const char *name;
    size_t size;
    vinit_cb vinit;
    init_copy_cb init_copy;
    destroy_cb destroy;
    repr_cb repr;
    to_cstr_cb to_cstr;
} Class;

const Class *class_of(const void *self);
const char *name_of(const void *self);

void *__new__(const Class *cls, ...);
void __init__(const Class *cls, void *self, ...);
void *new_copy(const void *other);
void init_copy(void *self, const void *other);
void destroy(void *self);
void delete(void *self);

size_t repr(const void *self, char *cstr, size_t size);
size_t to_cstr(const void *self, char *cstr, size_t size);


#endif /* _MASC_CLASS_H_ */
