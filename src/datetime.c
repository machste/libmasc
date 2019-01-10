#include <stdlib.h>
#include <stdio.h>

#include <masc/datetime.h>
#include <masc/math.h>
#include <masc/cstr.h>


DateTime *datetime_new(const char *date)
{
    DateTime *self = malloc(sizeof(DateTime));
    datetime_init(self, date);
    return self;
}

void datetime_init(DateTime *self, const char *date)
{
    object_init(self, DateTimeCls);
    if (date != NULL) {
        datetime_parse(self, date);
    } else {
        clock_gettime(CLOCK_REALTIME_COARSE, &self->ts);
        self->valid = true;
    }
}

void datetime_vinit(DateTime *self, va_list va)
{
    const char *date = va_arg(va, const char *);
    datetime_init(self, date);
}

DateTime *datetime_new_unix(time_t t)
{
    DateTime *self = malloc(sizeof(DateTime));
    datetime_init_unix(self, t);
    return self;
}

void datetime_init_unix(DateTime *self, time_t t)
{
    object_init(self, DateTimeCls);
    datetime_from_unix(self, t);
}

void datetime_delete(DateTime *self)
{
    free(self);
}

bool datetime_is_valid(DateTime *self)
{
    return self->valid;
}

void datetime_from_unix(DateTime *self, time_t t)
{
    self->ts.tv_sec = t;
    self->ts.tv_nsec = 0;
    self->valid = true;
}

time_t datetime_to_unix(DateTime *self)
{
    return self->ts.tv_sec;
}

int datetime_cmp(DateTime *self, DateTime *other)
{
    if (self->ts.tv_sec > other->ts.tv_sec) {
        return 1;
    } else if (self->ts.tv_sec < other->ts.tv_sec) {
        return -1;
    } else {
        if (self->ts.tv_nsec > other->ts.tv_nsec) {
            return 1;
        } else if (self->ts.tv_nsec < other->ts.tv_nsec) {
            return -1;
        } else {
            return 0;
        }
    }
}

size_t datetime_to_cstr(DateTime *self, char *cstr, size_t size)
{
    long len;
    if (datetime_is_valid(self)) {
        struct tm t;
        localtime_r(&self->ts.tv_sec, &t);
        len = snprintf(cstr, size, "%04i-%02i-%02i %02i:%02i:%02i",
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
                t.tm_sec);
        if (self->ts.tv_nsec != 0) {
            len += snprintf(cstr + len, max(0, size - len), ".%06i",
                    (int)(self->ts.tv_nsec / 1000));
        }
    } else {
        len = cstr_ncopy(cstr, "<invalid date>", size);
    }
    return len;
}


static class _DateTimeCls = {
    .name = "DateTime",
    .size = sizeof(DateTime),
    .vinit = (vinit_cb)datetime_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .cmp = (cmp_cb)datetime_cmp,
    .repr = (repr_cb)datetime_to_cstr,
    .to_cstr = (to_cstr_cb)datetime_to_cstr,
};

const class *DateTimeCls = &_DateTimeCls;
