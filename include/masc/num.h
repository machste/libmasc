#ifndef _MASC_NUM_H_
#define _MASC_NUM_H_

#include <masc/object.h>


typedef struct {
    Object obj;
    double val;
} Num;


extern const Class *NumCls;


Num *num_new(double val);
void num_init(Num *self, double val);

void num_delete(Num *self);

double num_get(Num *self);
void num_set(Num *self, double value);
double num_iadd(Num *self, Num *other);
Num *num_add(Num *self, Num *other);

size_t num_to_cstr(Num *self, char *cstr, size_t size);

#endif /* _MASC_NUM_H_ */
