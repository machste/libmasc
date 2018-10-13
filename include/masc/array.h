#ifndef _MASC_ARRAY_H_
#define _MASC_ARRAY_H_

#include <stdlib.h>
#include <stdbool.h>

#include <masc/object.h>


typedef struct {
    Object;
    size_t obj_size;
    size_t len;
    void *data;
} Array;


extern const class *ArrayCls;


Array *array_new(size_t obj_size, size_t len);
void array_init(Array *self, size_t obj_size, size_t len);

Array *array_new_copy(Array *other);
void array_init_copy(Array *self, Array *other);

void array_destroy(Array *self);
void array_delete(Array *self);

size_t array_len(Array *self);

size_t array_size(Array *self);

void *array_get_at(Array *self, int idx);
bool array_set_at(Array *self, int idx, Object *obj);
bool array_copy_at(Array *self, int idx, Object *obj);
void *array_remove_at(Array *self, int idx);
bool array_destroy_at(Array *self, int idx);

void array_for_each(Array *self, void (*obj_cb)(Object *));

size_t array_to_cstr(Array *self, char *cstr, size_t size);

#endif /* _MASC_ARRAY_H_ */
