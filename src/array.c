#include <string.h>

#include <masc/array.h>
#include <masc/math.h>
#include <masc/cstr.h>


Array *array_new(size_t obj_size, size_t len)
{
    Array *self = malloc(sizeof(Array));
    array_init(self, obj_size, len);
    return self;
}

void array_init(Array *self, size_t obj_size, size_t len)
{
    object_init(&self->obj, ArrayCls);
    self->obj_size = obj_size;
    self->len = len;
    self->data = calloc(len, obj_size);
}

static void _vinit(Array *self, va_list va)
{
    size_t obj_size = va_arg(va, size_t);
    size_t len = va_arg(va, size_t);
    array_init(self, obj_size, len);
}

Array *array_new_copy(Array *other)
{
    Array *self = malloc(sizeof(Array));
    array_init_copy(self, other);
    return self;
}

void array_init_copy(Array *self, Array *other)
{
    object_init(&self->obj, ArrayCls);
    self->obj_size = other->obj_size;
    self->len = other->len;
    size_t size = array_size(other);
    self->data = malloc(size);
    // Copy all objects
    void *s_obj = self->data;
    void *o_obj = other->data;
    for (int i = 0; i < self->len && i < other->len; i++) {
        if(class_of(o_obj) != NULL) {
            init_copy(s_obj, o_obj);
        }
        s_obj += self->obj_size;
        o_obj += other->obj_size;
    }    
}

void array_destroy(Array *self)
{
    array_for_each(self, destroy);
    free(self->data);
    self->len = 0;
}

void array_delete(Array *self)
{
    array_destroy(self);
    free(self);
}

size_t array_len(Array *self)
{
    return self->len;
}

size_t array_size(Array *self)
{
    return self->obj_size * self->len;
}

static int fixup_index(size_t len, int index)
{
    // Handle negative indexes (e.g. -1 equals to the last index)
    if (index < 0) {
        index += len;
    }
    // Check the range
    if (index < 0 || index >= len) {
        // Index could not be corrected!
        index = -1;
    }
    return index;
}

void *array_get_at(Array *self, int index)
{
    
    if ((index = fixup_index(self->len, index)) >= 0) {
        return self->data + self->obj_size * index;
    }
    return NULL;
}

bool array_set_at(Array *self, int index, void *obj)
{
    if ((index = fixup_index(self->len, index)) >= 0) {
        void *dest = self->data + self->obj_size * index;
        // Before setting a new object destroy the old one
        if (class_of(dest) != NULL) {
            destroy(dest);
        }
        memcpy(dest, obj, self->obj_size);
        return true;
    }
    return false;
}

bool array_copy_at(Array *self, int index, void *obj)
{
    if ((index = fixup_index(self->len, index)) >= 0) {
        void *dest = self->data + self->obj_size * index;
        // Before copy a new object destroy the old one
        if (class_of(dest) != NULL) {
            destroy(dest);
        }
        init_copy(dest, obj);
        return true;
    }
    return false;
}

bool array_destroy_at(Array *self, int index)
{
    if ((index = fixup_index(self->len, index)) >= 0) {
        void *dest = self->data + self->obj_size * index;
        if (class_of(dest) != NULL) {
            destroy(dest);
            // Mark the array index as empty
            ((Object *)dest)->cls = NULL;
        }
        return true;
    }
    return false;
}

static void *_next(Iter *itr, Array *self)
{
    void *obj = itr->ptr;
    itr->index++;
    if (itr->index < self->len) {
        itr->ptr += self->obj_size;
    } else {
        return NULL;
    }
    return obj;
}

static bool _is_last(Iter *itr, Array *self)
{
    return itr->index == self->len - 1;
}

Iter array_iter(Array *self)
{
    Iter i;
    iter_init(&i, self, (next_cb)_next, (is_last_cb)_is_last,
            self->data, -1, NULL);
    return i;
}

void array_for_each(Array *self, void (*obj_cb)(void *))
{
    if (obj_cb == NULL) {
        return;
    }
    void *obj = self->data;
    for (int i = 0; i < self->len; i++) {
        if(class_of(obj) != NULL) {
            obj_cb(obj);
        }
        obj += self->obj_size;
    }
}

size_t array_to_cstr(Array *self, char *cstr, size_t size)
{
    int len = 0;
    len += cstr_copy(cstr, "[", size);
    void *obj = self->data;
    for (int i = 0; i < self->len; i++) {
        if (class_of(obj) != NULL) {
            len += repr(obj, cstr + len, max(0, size - len));
        } else {
            len += cstr_copy(cstr + len, "null", max(0, size - len));
        }
        if (i < self->len - 1) {
            len += cstr_copy(cstr + len, ", ", max(0, size - len));
        }
        obj += self->obj_size;
    }
    len += cstr_copy(cstr + len, "]", max(0, size - len));
    return len;
}

static Class _ArrayCls = {
    .name = "Array",
    .size = sizeof(Array),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)array_init_copy,
    .destroy = (destroy_cb)array_destroy,
    .repr = (repr_cb)array_to_cstr,
    .to_cstr = (to_cstr_cb)array_to_cstr,
};

const Class *ArrayCls = &_ArrayCls;
