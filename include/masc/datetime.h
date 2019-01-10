#ifndef _MASC_DATETIME_H
#define _MASC_DATETIME_H

#include <time.h>
#include <stdbool.h>

#include <masc/object.h>


typedef struct {
    Object;
    struct timespec ts;
    bool valid;
} DateTime;


extern const class *DateTimeCls;


DateTime *datetime_new(const char *date);
void datetime_init(DateTime *self, const char *date);
void datetime_vinit(DateTime *self, va_list va);

DateTime *datetime_new_unix(time_t t);
void datetime_init_unix(DateTime *self, time_t t);

void datetime_delete(DateTime *self);

bool datetime_is_valid(DateTime *self);

bool datetime_parse(DateTime *self, const char *date);

void datetime_from_unix(DateTime *self, time_t t);
time_t datetime_to_unix(DateTime *self);

int datetime_cmp(DateTime *self, DateTime *other);

size_t datetime_to_cstr(DateTime *self, char *cstr, size_t size);

#endif /* _MASC_DATETIME_H */
