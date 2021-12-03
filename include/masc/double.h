#ifndef _MASC_DOUBLE_H_
#define _MASC_DOUBLE_H_

#include <stdbool.h>

#include <masc/num_class.h>


typedef struct {
    Num;
    double val;
} Double;


extern const num_class *DoubleCls;


Double *double_new(double val);
void double_init(Double *self, double val);

Double *double_new_cstr(const char *cstr, bool strict);
void double_init_cstr(Double *self, const char *cstr, bool strict);

void double_delete(Double *self);

double double_get(Double *self);
void double_set(Double *self, double value);
bool double_set_cstr(Double *self, const char *cstr, bool strict);

double double_iadd(Double *self, double other);
#define double_incr(self) double_iadd(self, 1)
Double *double_add(Double *self, double other);
#define double_isub(self, other) double_iadd(self, -(other))
#define double_decr(self) double_iadd(self, -1)
#define double_sub(self, other) double_add(self, -(other))

int double_cmp(const Double *self, const Double *other);

size_t double_to_cstr(Double *self, char *cstr, size_t size);
long double_to_int(Double *self);

#endif /* _MASC_DOUBLE_H_ */
