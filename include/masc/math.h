#ifndef _MASC_MATH_H_
#define _MASC_MATH_H_

#include <stdbool.h>

#include <masc/int.h>


long max(long a, long b);
long min(long a, long b);

Int *cstr_to_int(const char *cstr, bool strict, char **endptr);
void *cstr_to_number(const char *cstr, bool strict, char **endptr);

#endif /* _MASC_MATH_H_ */
