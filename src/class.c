#include <string.h>

#include <masc/class.h>
#include <masc/object.h>
#include <masc/iter.h>
#include <masc/none.h>
#include <masc/cstr.h>


void __class_init__(class *cls)
{
    if (cls->init_class != NULL) {
        // Init class
        cls->init_class(cls);
        // Class objects are singleton, ensure they are only initialised once.
        cls->init_class = NULL;
        object_init(cls, ClassCls);
        // Init super class
        if (cls->super != NULL) {
            __class_init__((class *)cls->super);
        }
    }
}

bool issubclass(const class *cls, const class *subcls)
{
    if (cls == subcls) {
        return true;
    } else if (cls == NULL) {
        return false;
    } else {
        return issubclass(cls->super, subcls);
    }
}

const class *class_of(const Object *obj)
{
    if (obj != NULL) {
        return obj->cls;
    } else {
        return NULL;
    }
}

const char *name_of(const Object *obj)
{
    if (obj != NULL && class_of(obj) != NULL) {
        return class_of(obj)->name;
    } else {
        return none_cstr;
    }
}

void *__new__(const class *cls, ...)
{
    va_list va;
    Object *self = malloc(cls->size);
    va_start(va, cls);
    cls->vinit(self, va);
    va_end(va);
    return self;
}

void __init__(const class *cls, Object *self, ...)
{
    va_list va;
    va_start(va, self);
    cls->vinit(self, va);
    va_end(va);
}

void *new_copy(const Object *other)
{
    if (other == NULL || class_of(other) == NULL) {
        return NULL;
    }
    const class *cls = class_of(other);
    Object *self = malloc(cls->size);
    cls->init_copy(self, other);
    return self;
}

void init_copy(Object *self, const Object *other)
{
    if (other == NULL || class_of(other) == NULL) {
        *(None *)self = init(None);
    } else {
        const class *cls = class_of(other);
        cls->init_copy(self, other);
    }
}

void destroy(Object *self)
{
    if (self == NULL || class_of(self) == NULL) {
        return;
    }
    class_of(self)->destroy(self);
    
}

void delete(Object *self)
{
    if (self == NULL || class_of(self) == NULL) {
        return;
    }
    class_of(self)->destroy(self);
    free(self);
}

void __delete_objs__(Object *self, ...)
{
    va_list va;
    va_start(va, self);
    while (self != NULL) {
        delete(self);
        self = va_arg(va, Object *);
    }
    va_end(va);
}

int cmp(const Object *self, const Object *other)
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

size_t repr(const Object *self, char *cstr, size_t size)
{
    if (self != NULL && class_of(self) != NULL) {
        return class_of(self)->repr(self, cstr, size);
    } else {
        return cstr_ncopy(cstr, none_repr, size);
    }
}

size_t to_cstr(const Object *self, char *cstr, size_t size)
{
    if (self != NULL && class_of(self) != NULL) {
        return class_of(self)->to_cstr(self, cstr, size);
    } else {
        return cstr_ncopy(cstr, none_cstr, size);
    }
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}

static void _vinit(class *self, va_list _)
{
    object_init(self, ClassCls);
}


static class _ClassCls = {
    .name = "Class",
    .size = sizeof(class),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = object_init_copy,
    .destroy = object_destroy,
    .cmp = object_cmp,
    .repr = object_to_cstr,
    .to_cstr = object_to_cstr,
};

const class *ClassCls = &_ClassCls;
const List *__classes__ = NULL;
