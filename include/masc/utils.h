#ifndef _MASC_UTILS_H_
#define _MASC_UTILS_H_

#include <stdlib.h>

typedef int (*qs_cmp_cb)(const void *, const void *, void *arg);

void quicksort(void *base, size_t nel, size_t width, qs_cmp_cb cmp, void *arg);

void hexdump(const void *data, size_t size);

#endif /* _MASC_UTILS_H_ */
