#include <masc/utils.h>


void quicksort(void *base, size_t nel, size_t width, qs_cmp_cb cb, void *arg)
{
    int inner_cmp(const void *p1, const void *p2) {
        return cb(p1, p2, arg);
    }
    qsort(base, nel, width, inner_cmp);
}
