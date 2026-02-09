#ifndef _MASC_POINTER_H_
#define _MASC_POINTER_H_

#include <masc/object.h>


typedef struct {
    Object;
    void *ptr;
} Pointer;


extern const class *PointerCls;


Pointer *pointer_new(void *ptr);
void pointer_init(Pointer *self, void *ptr);
void pointer_vinit(Pointer *self, va_list va);

void pointer_delete(Pointer *self);

void *pointer_get(Pointer *self);
void pointer_set(Pointer *self, void *ptr);

int pointer_cmp(const Pointer *self, const Pointer *other);

size_t pointer_to_cstr(Pointer *self, char *cstr, size_t size);

#endif /* _MASC_POINTER_H_ */
