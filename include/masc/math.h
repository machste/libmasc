#ifndef _MASC_MATH_H_
#define _MASC_MATH_H_

#include <stdbool.h>


long max(long a, long b);
long min(long a, long b);

void *cstr_to_number(const char *cstr, bool strict, char **endptr);

#endif /* _MASC_MATH_H_ */
