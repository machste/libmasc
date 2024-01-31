#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "masc/mtrace.h"
#include "masc/math.h"
#include "masc/utils.h"


#define STOP_TRACING() bool __bkp_tracing = tracing; tracing = false
#define RESUME_TRACING() tracing = __bkp_tracing;

#ifdef MEM_TRACER_DBG
#define PRINT_DBG(x, ...) printf("==MTRACE==: " x "\n", ##__VA_ARGS__)
#else
#define PRINT_DBG(x, ...) if(0) {}
#endif

#define PRINT_ERR(x, ...) printf("==MTRACE==: ERR: " x "\n", ##__VA_ARGS__)

static bool tracing = false;
static mtrace_entry_t *entries = NULL;
static size_t count = 0;
static size_t max_count = 0;


void *__real_malloc(size_t size);
void *__real_calloc(size_t nmemb, size_t size);
void *__real_realloc(void *ptr, size_t size);
char *__real_strdup(const char *s);
char *__real_strndup(const char *s, size_t n);
void __real_free(void *ptr);

static void _add_entry(void *ptr, size_t size)
{
    if (ptr == NULL || size == 0) {
        PRINT_ERR("got invalid memory block!");
        return;
    }
    if (count >= max_count) {
        PRINT_ERR("unable to add mtrace entry!");
        exit(EXIT_FAILURE);
    }
    // Look for next free slot
    for (size_t i = 0; i < max_count; i++) {
        if (entries[i].size == 0) {
            PRINT_DBG("add: %p, %zu", ptr, size);
            entries[i].ptr = ptr;
            entries[i].size = size;
            break;
        }
    }
    count++;
}

static void _remove_entry(void *ptr)
{
    // Look for the memory block
    for (size_t i = 0; i < max_count; i++) {
        if (entries[i].ptr == ptr) {
            PRINT_DBG("rm: %p, %zu", ptr, entries[i].size);
            entries[i].ptr = NULL;
            entries[i].size = 0;
            count--;
            return;
        }
    }
    PRINT_ERR("no mtrace found for memory block (%p)!", ptr);
}

static void _update_entry(void *ptr, void *new_ptr, size_t size)
{
    if (new_ptr == NULL) {
        PRINT_ERR("got invalid memory block!");
        return;
    }
    if (size == 0) {
        _remove_entry(ptr);
        return;
    }
    // Look for entry and update
    for (size_t i = 0; i < max_count; i++) {
        if (entries[i].ptr == ptr) {
            if (ptr != new_ptr) {
                PRINT_DBG("update: %p -> %p, %zu", ptr, new_ptr, size);
            } else {
                PRINT_DBG("update: %p, %zu", new_ptr, size);
            }
            entries[i].ptr = new_ptr;
            entries[i].size = size;
            break;
        }
    }
}

void *__wrap_malloc(size_t size)
{
    void *ptr = __real_malloc(size);
    if (tracing) {
        _add_entry(ptr, size);
    }
    return ptr;
}

void *__wrap_calloc(size_t nmemb, size_t size)
{
    void *ptr = __real_calloc(nmemb, size);
    if (tracing) {
        _add_entry(ptr, nmemb * size);
    }
    return ptr;
}

void *__wrap_realloc(void *ptr, size_t size)
{
    void *new_ptr = __real_realloc(ptr, size);
    if (tracing) {
        if (ptr == NULL) {
            _add_entry(new_ptr, size);
        } else {
            _update_entry(ptr, new_ptr, size);
        }
    }
    return new_ptr;
}

char *__wrap_strdup(const char *s)
{
    char *cstr = __real_strdup(s);
    if (tracing) {
        size_t size = strlen(s);
        _add_entry(cstr, size + 1);
    }
    return cstr;
}

char *__wrap_strndup(const char *s, size_t n)
{
    char *cstr = __real_strndup(s, n);
    if (tracing) {
        size_t size = max(strlen(s), n);
        _add_entry(cstr, size + 1);
    }
    return cstr;
}

void __wrap_free(void *ptr)
{
    if (ptr != NULL && tracing) {
        _remove_entry(ptr);
    }
    __real_free(ptr);
}

void mtrace_init(size_t n)
{
    tracing = false;
    entries = malloc(n * sizeof(mtrace_entry_t));
    if (entries != NULL) {
        memset(entries, 0, n * sizeof(mtrace_entry_t));
        count = 0;
        max_count = n;
        tracing = true;
    }
}

void mtrace_destroy(void)
{
    tracing = false;
    free(entries);
    count = 0;
    max_count = 0;
}

void mtrace_reset(void)
{
    if (entries == NULL) {
        return;
    }
    memset(entries, 0, max_count * sizeof(mtrace_entry_t));
    count = 0;
}

size_t mtrace_used(void)
{
    size_t used = 0;
    for (size_t i = 0; i < max_count; i++) {
        used += entries[i].size;
    }
    return used;
}

void mtrace_print_info(void)
{
    if (!tracing) {
        printf("==MTRACE==: no tracing!\n");
        return;
    }
    STOP_TRACING();
    // Print info of memmory tracer
    printf("==MTRACE==: count: %zu\n", count);
    printf("==MTRACE==: used: %zu\n", mtrace_used());
    for (size_t i = 0; i < max_count; i++) {
        if (entries[i].ptr != NULL) {
            printf("==MTRACE==:  * %p: %zu\n", entries[i].ptr, entries[i].size);
            hexdump_ind("==MTRACE==:      ", entries[i].ptr, entries[i].size);
        }
    }
    RESUME_TRACING();
}
