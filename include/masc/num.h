#ifndef _MASC_NUM_H_
#define _MASC_NUM_H_

#include <stdbool.h>

#include <masc/object.h>


typedef struct {
    Object;
    double val;
} Num;


extern const class *NumCls;


Num *num_new(double val);
void num_init(Num *self, double val);

Num *num_new_cstr(const char *cstr, bool strict);
void num_init_cstr(Num *self, const char *cstr, bool strict);

void num_delete(Num *self);

double num_get(Num *self);
void num_set(Num *self, double value);
bool num_set_cstr(Num *self, const char *cstr, bool strict);
double num_iadd(Num *self, Num *other);
Num *num_add(Num *self, Num *other);

int num_cmp(const Num *self, const Num *other);

size_t num_to_cstr(Num *self, char *cstr, size_t size);

#endif /* _MASC_NUM_H_ */
