#include <string.h>

#include <masc/class.h>
#include <masc/object.h>
#include <masc/iter.h>
#include <masc/none.h>
#include <masc/cstr.h>


static const char *const null_cstr = "NULL";


const Class *class_of(const void *self)
{
    if (self != NULL) {
        return ((Object *)self)->cls;
    } else {
        return NULL;
    }
}

const char *name_of(const void *self)
{
    if (self != NULL && class_of(self) != NULL) {
        return class_of(self)->name;
    } else {
        return null_cstr;
    }
}

void *__new__(const Class *cls, ...)
{
    va_list va;
    void *self = malloc(cls->size);
    va_start(va, cls);
    cls->vinit(self, va);
    va_end(va);
    return self;
}

void __init__(const Class *cls, void *self, ...)
{
    va_list va;
    va_start(va, self);
    cls->vinit(self, va);
    va_end(va);
}

void *new_copy(const void *other)
{
    if (other == NULL || class_of(other) == NULL) {
        return NULL;
    }
    const Class *cls = class_of(other);
    void *self = malloc(cls->size);
    cls->init_copy(self, other);
    return self;
}

void init_copy(void *self, const void *other)
{
    if (other == NULL || class_of(other) == NULL) {
        *(None *)self = init(None);
    } else {
        const Class *cls = class_of(other);
        cls->init_copy(self, other);
    }
}

void destroy(void *self)
{
    if (self == NULL || class_of(self) == NULL) {
        return;
    }
    class_of(self)->destroy(self);
    
}

void delete(void *self)
{
    if (self == NULL || class_of(self) == NULL) {
        return;
    }
    class_of(self)->destroy(self);
    free(self);
}

size_t len(const void *self)
{
    if (self == NULL || class_of(self) == NULL) {
        return 0;
    }
    return class_of(self)->len(self);
}

int cmp(const void *self, const void *other)
{
    if (self == other) {
        return 0;
    } else if (is_none(self) && !is_none(other)) {
        return -1;
    } else if (!is_none(self) && is_none(other)) {
        return 1;
    } else if (class_of(self) != class_of(other)) {
        return strcmp(name_of(self), name_of(other));
    } else {
        return class_of(self)->cmp(self, other);
    }
}

size_t repr(const void *self, char *cstr, size_t size)
{
    if (self != NULL && class_of(self) != NULL) {
        return class_of(self)->repr(self, cstr, size);
    } else {
        return cstr_ncopy(cstr, none_repr, size);
    }
}

size_t to_cstr(const void *self, char *cstr, size_t size)
{
    if (self != NULL && class_of(self) != NULL) {
        return class_of(self)->to_cstr(self, cstr, size);
    } else {
        return cstr_ncopy(cstr, null_cstr, size);
    }
}

void filter(void *iterable, filter_cb cb)
{
    Iter *itr = new(Iter, iterable);
    for (void *obj = next(itr); obj != NULL; obj = next(itr)) {
        if (!cb(obj)) {
            iter_del_obj(itr);
        }
    }
    delete(itr);
}
