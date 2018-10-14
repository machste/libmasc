#include <stdio.h>
#include <string.h>

#include <masc/regex.h>
#include <masc/str.h>
#include <masc/cstr.h>
#include <masc/math.h>


Regex *regex_new(const char *regex)
{
    Regex *self = malloc(sizeof(Regex));
    regex_init(self, regex);
    return self;
}

void regex_init(Regex *self, const char *regex)
{
    object_init(self, RegexCls);
    self->regex = strdup(regex);
    self->err = regcomp(&self->re, self->regex, REG_EXTENDED);
}

static void _vinit(Regex *self, va_list va)
{
    char * regex = va_arg(va, char *);
    regex_init(self, regex);
}

Regex *regex_new_copy(Regex *other)
{
    return regex_new(other->regex);
}

void regex_init_copy(Regex *self, Regex *other)
{
    regex_init(self, other->regex);
}

void regex_destroy(Regex *self)
{
    free(self->regex);
    regfree(&self->re);
}

void regex_delete(Regex *self)
{
    regex_destroy(self);
    free(self);
}

bool regex_is_valid(Regex *self)
{
    return self->err == 0;
}

size_t regex_get_nsub(Regex *self)
{
    if (self->err == 0) {
        return self->re.re_nsub;
    } else {
        return 0;
    }
}

bool regex_is_match(Regex *self, const char *cstr)
{
    if(self->err == 0) {
        return regexec(&self->re, cstr, 0, NULL, 0) == 0;
    }
    return false;
}

Str *regex_find(Regex *self, const char *cstr)
{
    Str *str = NULL;
    if(self->err == 0) {
        regmatch_t p;
        if (regexec(&self->re, cstr, 1, &p, 0) == 0) {
            str = str_new_ncopy(cstr + p.rm_so, p.rm_eo - p.rm_so);
        }
    }
    return str;
}

Array *regex_search(Regex *self, const char *cstr)
{
    Array *result = NULL;
    if(self->err == 0) {
        size_t n = self->re.re_nsub + 1;
        regmatch_t p[n];
        if (regexec(&self->re, cstr, n, p, 0) == 0) {
            result = array_new(sizeof(Str), n);
            for (int i = 0; i < n; i++) {
                if (p[i].rm_so == p[i].rm_eo) {
                    continue;
                }
                Str *s = array_get_at(result, i);
                str_init_ncopy(s, cstr + p[i].rm_so, p[i].rm_eo - p[i].rm_so);
            }
        }
    }
    return result;
}

List *regex_split(Regex *self, const char *cstr, int maxsplit)
{
    List *parts = NULL;
    if(self->err == 0) {
        size_t pos = 0;
        size_t split = 0;
        regmatch_t p;
        parts = new(List);
        while(maxsplit < 0 || split < maxsplit) {
            if (regexec(&self->re, cstr + pos, 1, &p, 0) == 0) {
                list_append(parts, str_new_ncopy(cstr + pos, p.rm_so));
                pos += p.rm_eo;
            } else {
                break;
            }
            split++;
        }
        list_append(parts, str_new_cstr(cstr + pos));
    }
    return parts;
}

size_t regex_to_cstr(Regex *self, char *cstr, size_t size)
{
    long l = 0;
    l += snprintf(cstr, size, "/%s/", self->regex);
    if (self->err != 0) {
        l += snprintf(cstr + l, max(0, size - l), " (Error %i: ",
                self->err);
        l += regerror(self->err, &self->re, cstr + l, max(0, size - l)) - 1;
        l += cstr_ncopy(cstr + l, ")", max(0, size - l));
    }
    return l;
}


static class _RegexCls = {
    .name = "Regex",
    .size = sizeof(Regex),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)regex_init_copy,
    .destroy = (destroy_cb)regex_destroy,
    .len = (len_cb)object_len,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)regex_to_cstr,
    .to_cstr = (to_cstr_cb)regex_to_cstr,
};

const class *RegexCls = &_RegexCls;
