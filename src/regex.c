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
    object_init(&self->obj, RegexCls);
    self->regex = strdup(regex);
    self->err = regcomp(&self->re, self->regex, REG_EXTENDED);
}

static void _vinit(Regex *self, va_list va)
{
    regex_init(self, va_arg(va, char *));
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

size_t regex_get_nsub(Regex *self)
{
    if (self->err == 0) {
        return self->re.re_nsub;
    } else {
        return 0;
    }
}

Array *regex_search(Regex *self, const char *cstr)
{
    Array *result = NULL;
    size_t nmatch = self->re.re_nsub + 1;
    regmatch_t pmatch[nmatch];
    if (regexec(&self->re, cstr, nmatch, pmatch, 0) == 0) {
        result = array_new(sizeof(Str), nmatch);
        for (int i = 0; i < nmatch; i++) {
            Str *str = array_get_at(result, i);
            str_init_ncopy(str, cstr + pmatch[i].rm_so,
					pmatch[i].rm_eo - pmatch[i].rm_so);
        }
    }
    return result;
}

size_t regex_to_cstr(Regex *self, char *cstr, size_t size)
{
    int len = 0;
    len += snprintf(cstr, size, "/%s/", self->regex);
    if (self->err != 0) {
        len += snprintf(cstr + len, max(0, size - len), " (Error %i: ",
                self->err);
        len += regerror(self->err, &self->re, cstr + len, max(0, size - len));
        len += cstr_ncopy(cstr + len, ")", max(0, size - len));
    }
    return len;
}


static Class _RegexCls = {
    .name = "Regex",
    .size = sizeof(Regex),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)regex_init_copy,
    .destroy = (destroy_cb)regex_destroy,
    .repr = (repr_cb)regex_to_cstr,
    .to_cstr = (to_cstr_cb)regex_to_cstr,
};

const Class *RegexCls = &_RegexCls;
