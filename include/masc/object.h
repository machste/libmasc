#ifndef _MASC_OBJECT_H_
#define _MASC_OBJECT_H_

#include <stdlib.h>
#include <stdarg.h>


#define new(cls, ...) __new__(cls ## Cls, ##__VA_ARGS__)
#define init(cls, self, ...) __init__(cls ## Cls, self, ##__VA_ARGS__)


typedef void (*vinit_cb)(void *self, va_list va);
typedef void (*init_copy_cb)(void *self, void *other);
typedef void (*destroy_cb)(void *self);

typedef size_t (*repr_cb)(void *self, char *cstr, size_t size);
typedef size_t (*to_cstr_cb)(void *self, char *cstr, size_t size);

typedef struct {
    const char *name;
    size_t size;
    vinit_cb vinit;
    init_copy_cb init_copy;
    destroy_cb destroy;
    repr_cb repr;
    to_cstr_cb to_cstr;
} Class;

typedef struct {
    const Class *cls;
} Object;


extern const void *ObjectCls;


void object_init(Object *self, const Class *cls);

void object_init_copy(Object *self, Object *other);

void object_destroy(Object *self);

size_t object_to_cstr(Object *self, char *cstr, size_t size);


const Class *class_of(void *self);
const char *name_of(void *self);

void *__new__(const Class *cls, ...);
void __init__(const Class *cls, void *self, ...);
void *new_copy(void *other);
void init_copy(void *self, void *other);
void destroy(void *self);
void delete(void *self);

size_t repr(void *self, char *cstr, size_t size);
size_t to_cstr(void *self, char *cstr, size_t size);

#endif /* _MASC_OBJECT_H_ */
