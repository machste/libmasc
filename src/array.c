#include <string.h>

#include <masc/array.h>
#include <masc/iter.h>
#include <masc/none.h>
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
    object_init(self, ArrayCls);
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
    object_init(self, ArrayCls);
    self->obj_size = other->obj_size;
    self->len = other->len;
    size_t size = array_size(other);
    self->data = malloc(size);
    // Copy all objects
    void *s_obj = self->data;
    void *o_obj = other->data;
    for (int i = 0; i < self->len && i < other->len; i++) {
        init_copy(s_obj, o_obj);
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

static int _fix_index(Array *self, int idx)
{
    size_t len = array_len(self);
        if (len == 0) {
        return -1;
    }
    // Handle negative indexes (e.g. -1 equals to the last index)
    if (idx < 0) {
        idx += len;
        if (idx < 0) {
            // Use first element
            idx = 0;
        }
    } else if (idx >= len) {
        // Use last element
        idx = len;
    }
    return idx;
}

void *array_get_at(Array *self, int idx)
{
    if ((idx = _fix_index(self, idx)) >= 0) {
        return self->data + self->obj_size * idx;
    }
    return NULL;
}

bool array_set_at(Array *self, int idx, Object *obj)
{
    if ((idx = _fix_index(self, idx)) >= 0) {
        Object *dest = self->data + self->obj_size * idx;
        // Before setting a new object destroy the old one
        destroy(dest);
        if(obj != NULL) {
            memcpy(dest, obj, self->obj_size);
        } else {
            *(None *)dest = init(None);
        }
        return true;
    }
    return false;
}

bool array_copy_at(Array *self, int idx, Object *obj)
{
    if ((idx = _fix_index(self, idx)) >= 0) {
        Object *dest = self->data + self->obj_size * idx;
        // Before copy a new object destroy the old one
        destroy(dest);
        init_copy(dest, obj);
        return true;
    }
    return false;
}

void *array_remove_at(Array *self, int idx)
{
    Object *obj = NULL;
    if ((idx = _fix_index(self, idx)) >= 0) {
        Object *dest = self->data + self->obj_size * idx;
        if (!is_none(dest)) {
            obj = new_copy(dest);
            destroy(dest);
            *(None *)dest = init(None);
        }
    }
    return obj;
}

bool array_destroy_at(Array *self, int idx)
{
    if ((idx = _fix_index(self, idx)) >= 0) {
        Object *dest = self->data + self->obj_size * idx;
        if (!is_none(dest)) {
            destroy(dest);
            *(None *)dest = init(None);
            return true;
        }
    }
    return false;
}

void array_for_each(Array *self, void (*obj_cb)(Object *))
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
    long len = 0;
    len += cstr_ncopy(cstr, "[", size);
    void *obj = self->data;
    for (int i = 0; i < self->len; i++) {
        if (class_of(obj) != NULL) {
            len += repr(obj, cstr + len, max(0, size - len));
        } else {
            len += cstr_ncopy(cstr + len, "null", max(0, size - len));
        }
        if (i < self->len - 1) {
            len += cstr_ncopy(cstr + len, ", ", max(0, size - len));
        }
        obj += self->obj_size;
    }
    len += cstr_ncopy(cstr + len, "]", max(0, size - len));
    return len;
}


typedef struct {
    void *ptr;
    int idx;
} iter_priv;

static void *_new_priv(Array *self)
{
    iter_priv *itr = malloc(sizeof(iter_priv));
    itr->ptr = self->data;
    itr->idx = -1;
    return itr;
}

static void *_next(Array *self, iter_priv *itr)
{
    void *obj = itr->ptr;
    itr->idx++;
    if (itr->idx < self->len) {
        itr->ptr += self->obj_size;
    } else {
        return NULL;
    }
    return obj;
}

static void _del_obj(Array *self, iter_priv *itr)
{
    array_destroy_at(self, itr->idx);
}

static bool _is_last(Array *self, iter_priv *itr)
{
    return itr->idx == self->len - 1;
}

static int _get_idx(Array *self, iter_priv *itr)
{
    return itr->idx;
}


static iterable_class _ArrayCls = {
    .name = "Array",
    .size = sizeof(Array),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)array_init_copy,
    .destroy = (destroy_cb)array_destroy,
    .repr = (repr_cb)array_to_cstr,
    .cmp = (cmp_cb)object_cmp,
    .to_cstr = (to_cstr_cb)array_to_cstr,
    // Interable Class
    .len = (len_cb)array_len,
    .new_priv = (new_priv_cb)_new_priv,
    .next = (next_cb)_next,
    .del_obj = (del_obj_cb)_del_obj,
    .is_last = (is_last_cb)_is_last,
    .get_idx = (get_idx_cb)_get_idx,
    .get_key = NULL,
    .delete_priv = free,
};

const class *ArrayCls = &_ArrayCls;
