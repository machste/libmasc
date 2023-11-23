#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <masc/str.h>
#include <masc/char.h>
#include <masc/cstr.h>
#include <masc/math.h>
#include <masc/regex.h>
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
    object_init(self, StrCls);
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
    object_init(self, StrCls);
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
    object_init(self, StrCls);
    self->size = len + 1;
    self->cstr = malloc(self->size);
    if (cstr != NULL) {
        memcpy(self->cstr, cstr, self->size);
        self->cstr[len] = '\0';
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

bool str_is_empty(Str *self)
{
    return cstr_is_empty(self->cstr);
}

char *str_cstr(Str *self)
{
    return self->cstr;
}

bool str_eq(Str *self, Str *other)
{
    return strcmp(self->cstr, other->cstr) == 0;
}

bool str_eq_cstr(Str *self, const char *other)
{
    return strcmp(self->cstr, other) == 0;
}

int str_cmp(Str *self, Str *other)
{
    return strcmp(self->cstr, other->cstr);
}

bool str_startswith(Str *self, const char *start)
{
    if (cstr_is_empty(start)) {
        return true;
    } else {
        return cstr_startswith(self->cstr, start) > 0;
    }
}

bool str_endswith(Str *self, const char *end)
{
    if (cstr_is_empty(end)) {
        return true;
    } else {
        return cstr_endswith(self->cstr, end) > 0;
    }
}

char str_get_at(Str *self, size_t idx)
{
    idx = _fix_index(self, idx);
    return self->cstr[idx];
}

void str_set_at(Str *self, size_t idx, char c)
{
    idx = _fix_index(self, idx);
    self->cstr[idx] = c;
}

void str_delete_at(Str *self, size_t idx)
{
    idx = _fix_index(self, idx);
    if (str_len(self) > 0) {
        cstr_ncopy(self->cstr + idx, self->cstr + idx + 1, self->size - idx);
        self->size--;
        self->cstr = realloc(self->cstr, self->size);
    }
}

Str *str_copy(Str *self, const char *cstr)
{
    size_t len = strlen(cstr);
    self->size = len + 1;
    self->cstr = realloc(self->cstr, self->size);
    strcpy(self->cstr, cstr);
    return self;
}

Str *str_prepend(Str *self, const char *cstr)
{
    size_t len = strlen(cstr);
    if (len > 0) {
        // Calculate new size
        self->size += len;
        self->cstr = realloc(self->cstr, self->size);
        for (long i = self->size - 1; i >= len; i--) {
            self->cstr[i] = self->cstr[i - len];
        }
        strncpy(self->cstr, cstr, len);
    }
    return self;
}

Str *str_prepend_fmt(Str *self, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    Str new;
    str_vinit(&new, fmt, va);
    va_end(va);
    str_prepend(self, new.cstr);
    str_destroy(&new);
    return self;
}

Str *str_append(Str *self, const char *cstr)
{
    size_t len = strlen(cstr);
    if (len > 0) {
        // Calculate new size
        size_t self_len = str_len(self);
        self->size += len;
        self->cstr = realloc(self->cstr, self->size);
        strcpy(self->cstr + self_len, cstr);
    }
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
    cstr_lower(self->cstr);
    return self;
}

Str *str_upper(Str *self)
{
    cstr_upper(self->cstr);
    return self;
}

Str *str_swapcase(Str *self)
{
    cstr_swapcase(self->cstr);
    return self;
}

Str *str_snakecase(Str *self)
{
    for (char *c = self->cstr; *c != '\0'; c++) {
        if (isalnum(*c)) {
            *c = tolower(*c);
        } else {
            *c = '_';
        }
    }
    return self;
}

Str *str_escape(Str *self)
{
    size_t new_size = cstr_escape(NULL, self->cstr, self->size - 1, 0) + 1;
    char *new_cstr = malloc(new_size);
    cstr_escape(new_cstr, self->cstr, self->size - 1, new_size);
    free(self->cstr);
    self->size = new_size;
    self->cstr = new_cstr;
    return self;
}

Str *str_unescape(Str *self)
{
    char *tmp_str = strdup(self->cstr);
    self->size = cstr_unescape(self->cstr, tmp_str, self->size) + 1;
    free(tmp_str);
    self->cstr = realloc(self->cstr, self->size);
    return self;
}

Str *str_unrepr(Str *self)
{
    char *tmp_str = strdup(self->cstr);
    self->size = cstr_unrepr(self->cstr, tmp_str, self->size) + 1;
    free(tmp_str);
    self->cstr = realloc(self->cstr, self->size);
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
    ssize_t i;
    for (i = self->size - 2; i >= 0 && isspace(self->cstr[i]); i--);
    self->size = i + 2;
    self->cstr = realloc(self->cstr, self->size);
    size_t j;
    for (j = 0; j < i + 1; j++) {
        self->cstr[j] = self->cstr[j];
    }
    self->cstr[j] = '\0';
    return self;
}

Str *str_strip(Str *self)
{
    size_t l, i;
    for (l = 0; l < self->size - 1 && isspace(self->cstr[l]); l++);
    ssize_t r;
    for (r = self->size - 2; r > l && isspace(self->cstr[r]); r--);
    self->size = r - l + 2;
    for (i = 0; i < self->size - 1; i++) {
        self->cstr[i] = self->cstr[l++];
    }
    self->cstr[i] = '\0';
    self->cstr = realloc(self->cstr, self->size);
    return self;
}

Str *str_truncate(Str *self, size_t len, const char *end)
{
    if (len >= self->size - 1) {
        return self;
    }
    self->size = len + 1;
    self->cstr = realloc(self->cstr, self->size);
    size_t end_len = (end != NULL) ? strlen(end) : 0;
    for (size_t i = 1; i <= end_len; i++) {
        ssize_t str_idx = len - i;
        if (str_idx >= 0) {
            self->cstr[str_idx] = end[end_len - i];
        }
    }
    self->cstr[len] = '\0';
    return self;
}

Str *str_ljust(Str *self, int width, char fillchar)
{
    if (width >= self->size) {
        size_t len = str_len(self);
        self->size = width + 1;
        self->cstr = realloc(self->cstr, self->size);
        for (size_t i = len; i < width; i++) {
            self->cstr[i] = fillchar;
        }
        self->cstr[width] = '\0';
    }
    return self;
}

Str *str_rjust(Str *self, int width, char fillchar)
{
    if (width >= self->size) {
        size_t len = str_len(self);
        size_t lspace = width - len;
        self->size = width + 1;
        self->cstr = realloc(self->cstr, self->size);
        for (int i = width - 1; i >= 0; i--) {
            if (i < len) {
                self->cstr[i + lspace] = self->cstr[i];
            }
            if (i < lspace) {
                self->cstr[i] = fillchar;
            }
        }
        self->cstr[width] = '\0';
    }
    return self;
}

Str *str_center(Str *self, int width, char fillchar)
{
    if (width >= self->size) {
        size_t len = str_len(self);
        size_t lspace = (width - len) / 2;
        self->size = width + 1;
        self->cstr = realloc(self->cstr, self->size);
        for (int i = width - 1; i >= 0; i--) {
            if (i < len) {
                self->cstr[i + lspace] = self->cstr[i];
            }
            if (i < lspace || i >= lspace + len) {
                self->cstr[i] = fillchar;
            }
        }
        self->cstr[width] = '\0';
    }
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
    return cstr_split(self->cstr, sep, maxsplit);
}

Str *str_join(void *iterable, const char *sep) {
    Iter *itr = new(Iter, iterable);
    Str *str = str_new_cstr("");
    for (void *obj = next(itr); obj != NULL; obj = next(itr)) {
        if (iter_is_last(itr)) {
            str_append_fmt(str, "%O", obj);
        } else {
            str_append_fmt(str, "%O%s", obj, sep);
        }
    }
    delete(itr);
    return str;
}

Num *str_to_number(Str *self, bool strict)
{
    return cstr_to_number(self->cstr, strict, NULL);
}

Int *str_to_int(Str *self, bool strict)
{
    return cstr_to_int(self->cstr, strict, NULL);
}

int str_find(Str *self, const char *cstr)
{
    char *pos = strstr(self->cstr, cstr);
    if (pos != NULL) {
        return pos - self->cstr;
    }
    return -1;
}

Str *str_replace(Str *self, const char *old, const char *now, int count)
{
    Str *str;
    List *parts = str_split(self, old, count);
    if (len(parts) > 1) {
        str = str_join(parts, now);
    } else {
        str = new_copy(self);
    }
    delete(parts);
    return str;
}

bool str_is_match(Str *self, const char *regex)
{
    bool ret = false;
    Regex re;
    regex_init(&re, regex);
    if (regex_is_valid(&re)) {
        ret = regex_is_match(&re, self->cstr);
    }
    regex_destroy(&re);
    return ret;
}

size_t str_repr(Str *self, char *cstr, size_t size)
{
    return cstr_repr(cstr, self->cstr, self->size - 1, size);
}

size_t str_to_cstr(Str *self, char *cstr, size_t size)
{
    size_t len = str_len(self);
    cstr_ncopy(cstr, self->cstr, size);
    return len;
}

Str *to_str(const void *obj)
{
    if (class_of(obj) == StrCls) {
        return str_new_copy(obj);
    } else {
        size_t len = to_cstr(obj, NULL, 0);
        Str *s = str_new_ncopy(NULL, len);
        to_cstr(obj, s->cstr, s->size);
        return s;
    }
}


typedef struct {
    Char c;
    int idx;
} iter_priv;

static void *_new_priv(Str *self)
{
    iter_priv *itr = malloc(sizeof(iter_priv));
    itr->c = init(Char, '\0');
    itr->idx = -1;
    return itr;
}

static void _delete_priv(iter_priv *itr)
{
    destroy(&itr->c);
    free(itr);
}

static void *_next(Str *self, iter_priv *itr)
{
    int i = ++itr->idx;
    if (i < self->size - 1) {
        itr->c.c = self->cstr[i];
        return &itr->c;
    }
    return NULL;
}

static void _del_obj(Str *self, iter_priv *itr)
{
    str_delete_at(self, itr->idx);
    itr->idx--;
}

static bool _is_last(Str *self, iter_priv *itr)
{
    return itr->idx == self->size - 2;
}

static int _get_idx(Str *self, iter_priv *itr)
{
    return itr->idx;
}

static void _init_class(class *cls)
{
    cls->super = IterableCls;
}


static iterable_class _StrCls = {
    .name = "Str",
    .size = sizeof(Str),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)str_init_copy,
    .destroy = (destroy_cb)str_destroy,
    .cmp = (cmp_cb)str_cmp,
    .repr = (repr_cb)str_repr,
    .to_cstr = (to_cstr_cb)str_to_cstr,
    // Interable Class
    .len = (len_cb)str_len,
    .new_priv = (new_priv_cb)_new_priv,
    .next = (next_cb)_next,
    .del_obj = (del_obj_cb)_del_obj,
    .is_last = (is_last_cb)_is_last,
    .get_idx = (get_idx_cb)_get_idx,
    .get_key = NULL,
    .delete_priv = (delete_priv_cb)_delete_priv,
};

const class *StrCls = &_StrCls;
