#include <stdlib.h>
#include <strings.h>

#include <masc/datetime.h>
#include <masc/regex.h>


bool datetime_parse(DateTime *self, const char *date)
{
    self->ts.tv_sec = 0;
    self->ts.tv_nsec = 0;
    self->valid = false;
    Regex re = init(Regex, "([0-9]{1,4})-([0-9]{1,2})-([0-9]{1,2})"
            "(\\s+([0-9]{1,2}):([0-9]{2})(:([0-9]{2})(\\.([0-9]{6}))?)?)?");
    Array *match = regex_search(&re, date);
    if (match != NULL) {
        struct tm t;
        bzero(&t, sizeof(t));
        t.tm_year = atoi(str_cstr((Str *)array_get_at(match, 1))) - 1900;
        t.tm_mon = atoi(str_cstr((Str *)array_get_at(match, 2))) - 1;
        if (t.tm_mon < 0 || t.tm_mon > 11) {
            goto error;
        }
        t.tm_mday = atoi(str_cstr((Str *)array_get_at(match, 3)));
        if (t.tm_mday < 1 || t.tm_mday > 31) {
            goto error;
        }
        Str *hour = array_get_at(match, 5);
        if (isinstance(hour, Str)) {
            t.tm_hour = atoi(str_cstr(hour));
            if (t.tm_hour < 0 || t.tm_hour > 23) {
                goto error;
            }
            t.tm_min = atoi(str_cstr((Str *)array_get_at(match, 6)));
            if (t.tm_min < 0 || t.tm_min > 59) {
                goto error;
            }
            Str *sec = array_get_at(match, 8);
            if (isinstance(sec, Str)) {
                t.tm_sec = atoi(str_cstr(sec));
                if (t.tm_sec < 0 || t.tm_sec > 59) {
                    goto error;
                }
                Str *nsec = array_get_at(match, 10);
                if (isinstance(nsec, Str)) {
                    self->ts.tv_nsec = atoi(str_cstr(nsec)) * 1000;
                }
            }
        }
        t.tm_isdst = -1;
        self->ts.tv_sec = mktime(&t);
        self->valid = true;
    error:
        delete(match);
    }
    destroy(&re);
    return datetime_is_valid(self);
}
