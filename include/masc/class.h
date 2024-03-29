#ifndef _MASC_CLASS_H_
#define _MASC_CLASS_H_

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include <masc/object.h>

#define new(cls, ...) __new__(cls##Cls, ##__VA_ARGS__)
#define init(cls, ...) ({cls s; __init__(cls##Cls, &s, ##__VA_ARGS__);s;})

#define delete_objs(obj, ...) __delete_objs__(obj, ##__VA_ARGS__, NULL)

#define isinstance(obj, cls) issubclass(class_of(obj), (const class *)cls##Cls)


typedef void (*init_class_cb)(class *cls);
typedef void (*vinit_cb)(Object *self, va_list va);
typedef void (*init_copy_cb)(Object *self, const Object *other);
typedef void (*destroy_cb)(Object *self);

typedef int (*cmp_cb)(const Object *self, const Object *other);

typedef size_t (*repr_cb)(const Object *self, char *cstr, size_t size);
typedef size_t (*to_cstr_cb)(const Object *self, char *cstr, size_t size);

struct class {
    Object;
    const char *name;
    size_t size;
    const class *super;
    init_class_cb init_class;
    vinit_cb vinit;
    init_copy_cb init_copy;
    destroy_cb destroy;
    cmp_cb cmp;
    repr_cb repr;
    to_cstr_cb to_cstr;
};


extern const class *ClassCls;

void __class_init__(class *cls);
bool issubclass(const class *cls, const class *other);

const class *class_of(const Object *obj);
const char *name_of(const Object *obj);

void *__new__(const class *cls, ...);
void __init__(const class *cls, Object *self, ...);
void *new_copy(const Object *other);
void init_copy(Object *self, const Object *other);
void destroy(Object *self);
void delete(Object *self);
void __delete_objs__(Object *self, ...);

int cmp(const Object *self, const Object *other);

size_t repr(const Object *self, char *cstr, size_t size);
size_t to_cstr(const Object *self, char *cstr, size_t size);


#endif /* _MASC_CLASS_H_ */
