#include <stdlib.h>

#include <masc/math.h>
#include <masc/num.h>
#include <masc/int.h>


long max(long a, long b)
{
    return a > b ? a : b;
}

long min(long a, long b)
{
    return a < b ? a : b;
}

void *cstr_to_number(const char *cstr, bool strict, char **endptr)
{
    void *num_obj = NULL;
    char *dend, *lend;
    double d = strtod(cstr, &dend);
    long l = strtol(cstr, &lend, 0);
    if (endptr != NULL) { *endptr = (char *)cstr; }
    if (dend != cstr) {
        if (dend == lend && (!strict || (strict && *lend == '\0'))) {
            num_obj = int_new(l);
            if (endptr != NULL) { *endptr = lend; }
        } else if (!strict || (strict && *dend == '\0')) {
            num_obj = num_new(d);
            if (endptr != NULL) { *endptr = dend; }
        }
    }
    return num_obj;
}
