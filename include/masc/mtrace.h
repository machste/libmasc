#ifndef _MASC_MTRACE_H_
#define _MASC_MTRACE_H_

#include <stdlib.h>


typedef struct {
    void *ptr;
    size_t size;
} mtrace_entry_t;


void mtrace_init(size_t max_entries);
void mtrace_destroy(void);

void mtrace_reset(void);

size_t mtrace_used(void);

void mtrace_print_info(void);

#endif /* _MASC_MTRACE_H_ */
