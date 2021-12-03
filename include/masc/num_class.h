#ifndef _MASC_NUM_CLASS_H_
#define _MASC_NUM_CLASS_H_

#include <unistd.h>
#include <stdbool.h>

#include <masc/object.h>


typedef struct Num {
    Object;
} Num;

typedef int (*to_int_cb)(Num *self);
typedef double (*to_double_cb)(Num *self);
typedef Num *(*add_cb)(Num *self, Num *other);
typedef void (*iadd_cb)(Num *self, Num *other);

typedef struct {
    class;
    to_int_cb to_int;
    to_double_cb to_double;
    add_cb add;
    iadd_cb iadd;
} num_class;

const num_class *num_class_of(const Num *self);

long to_int(Num *self);
double to_double(Num *self);
void iadd(Num *self, Num *other);
Num *add(Num *self, Num *other);

#endif /* _MASC_NUM_CLASS_H_ */
