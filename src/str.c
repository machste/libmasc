#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <masc/str.h>
#include <masc/cstr.h>
#include <masc/math.h>
#include <masc/print.h>


Str *str_new(const char *fmt, ...)
{
    va_list va;
    Str *self = malloc(sizeof(Str));
    va_start(va, fmt);
    str_vinit(self, fmt, va);
    va_end(va);
    return self;
}

void str_init(Str *self, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    str_vinit(self, fmt, va);
    va_end(va);
}

void str_vinit(Str *self, const char *fmt, va_list va)
{
    int len;
    va_list va2;
    object_init(&self->obj, StrCls);
    // Make a copy of va to use it twice
    va_copy(va2, va);
    // Calculate length of the formatted string
    len = vformat(NULL, 0, fmt, va);
    self->size = len + 1;
    self->cstr = malloc(self->size);
    // Do the actual work
    vformat(self->cstr, self->size, fmt, va2);
    va_end(va2);
}

static void _vinit(Str *self, va_list va)
{
    char *fmt = va_arg(va, char *);
    str_vinit(self, fmt, va);
}

Str *str_new_cstr(const char *cstr)
{
    Str *self = malloc(sizeof(Str));
    str_init_cstr(self, cstr);
    return self;
}

void str_init_cstr(Str *self, const char *cstr)
{
    object_init(&self->obj, StrCls);
    self->size = strlen(cstr) + 1;
    self->cstr = malloc(self->size);
    strcpy(self->cstr, cstr);
}

Str *str_new_ncopy(const char* cstr, size_t len)
{
    Str *self = malloc(sizeof(Str));
    str_init_ncopy(self, cstr, len);
    return self;
}

void str_init_ncopy(Str *self, const char* cstr, size_t len)
{
    object_init(&self->obj, StrCls);
    self->size = len + 1;
    self->cstr = malloc(self->size);
    if (cstr != NULL) {
        cstr_ncopy(self->cstr, cstr, self->size);
    } else {
        self->cstr[0] = '\0';
    }
}

Str *str_new_copy(const Str *other)
{
    return str_new_cstr(other->cstr);
}

void str_init_copy(Str *self, const Str *other)
{
    str_init_cstr(self, other->cstr);
}

static size_t _fix_index(Str *self, int index)
{
    // Handle negative indexes (e.g. -1 equals to the last index)
    size_t len = str_len(self);
    if (index < 0) {
        index += len;
        if (index < 0) {
            // Use first element
            index = 0;
        }
    } else if (index >= len) {
        // Use last element
        index = len;
    }
    return index;
}

Str *str_new_slice(Str *other, size_t start, size_t end)
{
    Str *self = malloc(sizeof(Str));
    str_init_slice(self, other, start, end);
    return self;
}

void str_init_slice(Str *self, Str *other, size_t start, size_t end)
{
    start = _fix_index(other, start);
    end = _fix_index(other, end);
    size_t len = max(0, end - start);
    str_init_ncopy(self, other->cstr + start, len);
}

void str_destroy(Str *self)
{
    if (self->cstr != NULL) {
        free(self->cstr);
        self->cstr = NULL;
    }
    self->size = 0;
}

void str_delete(Str *self)
{
    str_destroy(self);
    free(self);
}

size_t str_len(Str *self)
{
    return self->size > 0 ? self->size - 1 : 0;
}

char *str_cstr(Str *self)
{
    return self->cstr;
}

size_t str_repr(Str *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "\"%s\"", self->cstr);
}

size_t str_to_cstr(Str *self, char *cstr, size_t size)
{
    size_t len = str_len(self);
    cstr_ncopy(cstr, self->cstr, size);
    return len;
}

static void *_next(Iter *itr, Str *self)
{
    itr->index++;
    if (itr->index < self->size - 1) {
        return self->cstr + itr->index;
    }
    return NULL;
}

static bool _is_last(Iter *itr, Str *self)
{
    return itr->index + 2 == self->size;
}

Iter str_iter(Str *self)
{
    Iter i;
    iter_init(&i, self, (next_cb)_next, (is_last_cb)_is_last, NULL, -1, NULL);
    return i;
}

char str_get_at(Str *self, size_t index)
{
    index = _fix_index(self, index);
    return self->cstr[index];
}

void str_set_at(Str *self, size_t index, char c)
{
    index = _fix_index(self, index);
    self->cstr[index] = c;
}

Str *str_copy(Str *self, const char *cstr)
{
    size_t len = strlen(cstr);
    self->size = len + 1;
    self->cstr = realloc(self->cstr, self->size);
    strcpy(self->cstr, cstr);
    return self;
}

Str *str_append(Str *self, const char *cstr)
{
    size_t len = strlen(cstr);
    size_t self_len = str_len(self);
    // Calculate new size
    self->size += len;
    self->cstr = realloc(self->cstr, self->size);
    strcpy(self->cstr + self_len, cstr);
    return self;
}

Str *str_append_fmt(Str *self, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    Str new;
    str_vinit(&new, fmt, va);
    va_end(va);
    str_append(self, new.cstr);
    str_destroy(&new);
    return self;
}

Str *str_fmt(Str *self, const char *fmt, ...)
{
    va_list va1, va2;
    va_start(va1, fmt);
    // Make a copy of va to use it for the second run
    va_copy(va2, va1);
    // Calculate length of the formatted string
    int len = vformat(NULL, 0, fmt, va1);
    va_end(va1);
    self->size = len + 1;
    self->cstr = realloc(self->cstr, self->size);
    // Do the actual work
    vformat(self->cstr, self->size, fmt, va2);
    va_end(va2);
    return self;
}

Str *str_lower(Str *self)
{
    for (size_t i = 0; i < self->size - 1; i++) {
        self->cstr[i] = tolower(self->cstr[i]);
    }
    return self;
}

Str *str_upper(Str *self)
{
    for (size_t i = 0; i < self->size - 1; i++) {
        self->cstr[i] = toupper(self->cstr[i]);
    }
    return self;
}

Str *str_swapcase(Str *self)
{
    for (size_t i = 0; i < self->size - 1; i++) {
        if (islower(self->cstr[i])) {
            self->cstr[i] = toupper(self->cstr[i]);
        } else if (isupper(self->cstr[i])) {
            self->cstr[i] = tolower(self->cstr[i]);
        } else {
            self->cstr[i] = self->cstr[i];
        }
    }
    return self;
}

Str *str_reverse(Str *self)
{
    size_t half_len = str_len(self) / 2;
    for (size_t i = 0; i < half_len; i++) {
        char tmp = self->cstr[i];
        self->cstr[i] = self->cstr[self->size - i - 2];
        self->cstr[self->size - i - 2] = tmp;
    }
    return self;
}

Str *str_lstrip(Str *self)
{
    size_t i, j;
    for (i = 0; i < self->size - 1 && isspace(self->cstr[i]); i++);
    self->size -= i;
    for (j = 0; j < self->size - 1; j++) {
        self->cstr[j] = self->cstr[i++];
    }
    self->cstr[j] = '\0';
    self->cstr = realloc(self->cstr, self->size);
    return self;
}

Str *str_rstrip(Str *self)
{
    size_t i, j;
    for (i = self->size - 2; i >= 0 && isspace(self->cstr[i]); i--);
    self->size = i + 2;
    self->cstr = realloc(self->cstr, self->size);
    for (j = 0; j < i + 1; j++) {
        self->cstr[j] = self->cstr[j];
    }
    self->cstr[j] = '\0';
    return self;
}

Str *str_strip(Str *self)
{
    size_t l, r, i;
    for (l = 0; l < self->size && isspace(self->cstr[l]); l++);
    for (r = self->size - 2; r >= 0 && isspace(self->cstr[r]); r--);
    self->size = r - l + 2;
    
    for (i = 0; i < self->size - 1; i++) {
        self->cstr[i] = self->cstr[l++];
    }
    self->cstr[i] = '\0';
    self->cstr = realloc(self->cstr, self->size);
    return self;
}

Str *str_slice(Str *self, size_t start, size_t end) {
    start = _fix_index(self, start);
    end = _fix_index(self, end);
    self->size = max(0, end - start) + 1;
    char *old_cstr = self->cstr;
    self->cstr = malloc(self->size);
    cstr_ncopy(self->cstr, old_cstr + start, self->size);
    free(old_cstr);
    return self;
}

List *str_split(Str *self, const char *sep, int maxsplit)
{
    List *l = list_new();
    char *start = self->cstr, *end = self->cstr;
    size_t sep_len = strlen(sep);
    size_t split = 0;
    do {
        Str *slice;
        if (maxsplit < 0 || split < maxsplit) {
            end = strstr(start, sep);
        } else {
            end = NULL;
        }
        if (end != NULL) {
            slice = str_new_ncopy(start, end - start);
        } else {
            slice = str_new_cstr(start);
        }
        list_append(l, slice);
        start = end + sep_len;
        split++;
    } while (end != NULL);
    return l;
}

Str *str_join(List *objs, const char *sep) {
    Iter i = list_iter(objs);
    Str *str = str_new_cstr("");
    for (void *obj = next(&i); obj != NULL; obj = next(&i)) {
        if (is_last(&i)) {
            str_append_fmt(str, "%O", obj);
        } else {
            str_append_fmt(str, "%O%s", obj, sep);
        }
    }
    return str;
}

Str *to_str(const void *self)
{
    if (class_of(self) == StrCls) {
        return str_new_copy(self);
    } else {
        size_t len = to_cstr(self, NULL, 0);
        Str *s = str_new_ncopy(NULL, len);
        to_cstr(self, s->cstr, s->size);
        return s;
    }
}


static Class _StrCls = {
    .name = "Str",
    .size = sizeof(Str),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)str_init_copy,
    .destroy = (destroy_cb)str_destroy,
    .repr = (repr_cb)str_repr,
    .to_cstr = (to_cstr_cb)str_to_cstr,
};

const Class *StrCls = &_StrCls;
