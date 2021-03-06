#ifndef _MASC_REGEX_H_
#define _MASC_REGEX_H_

#include <regex.h>
#include <stdbool.h>

#include <masc/object.h>
#include <masc/str.h>
#include <masc/array.h>
#include <masc/list.h>


typedef struct {
    Object;
    char *regex;
    regex_t re;
    int err;
} Regex;


extern const class *RegexCls;


Regex *regex_new(const char *regex);
void regex_init(Regex *self, const char *regex);

Regex *regex_new_copy(Regex *other);
void regex_init_copy(Regex *self, Regex *other);

void regex_destroy(Regex *self);
void regex_delete(Regex *self);

bool regex_is_valid(Regex *self);

size_t regex_get_nsub(Regex *self);

bool regex_is_match(Regex *self, const char *cstr);
Str *regex_find(Regex *self, const char *cstr);
Array *regex_search(Regex *self, const char *cstr);

List *regex_split(Regex *self, const char *cstr, int maxsplit);

size_t regex_to_cstr(Regex *self, char *cstr, size_t size);

#endif /* _MASC_REGEX_H_ */
