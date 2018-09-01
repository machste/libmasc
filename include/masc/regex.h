#ifndef _MASC_REGEX_H_
#define _MASC_REGEX_H_

#include <regex.h>

#include <masc/object.h>
#include <masc/str.h>
#include <masc/array.h>


typedef struct {
    Object obj;
    char *regex;
    regex_t re;
    int err;
} Regex;


extern const Class *RegexCls;


Regex *regex_new(const char *regex);
void regex_init(Regex *self, const char *regex);

Regex *regex_new_copy(Regex *other);
void regex_init_copy(Regex *self, Regex *other);

void regex_destroy(Regex *self);
void regex_delete(Regex *self);

size_t regex_get_nsub(Regex *self);

Str *regex_find(Regex *self, const char *cstr);
Array *regex_search(Regex *self, const char *cstr);

size_t regex_to_cstr(Regex *self, char *cstr, size_t size);

#endif /* _MASC_REGEX_H_ */
