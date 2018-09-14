#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <masc/cstr.h>
#include <masc/math.h>
#include <masc/macro.h>


size_t cstr_putc(char *dest, const char c, size_t size)
{
    if (size > 0 && dest != NULL) {
        if (size > 1) {
            dest[0] = c;
            dest[1] = '\0';
        } else {
            dest[0] = '\0';
        }
    }
    return 1;
}

size_t cstr_ncopy(char *dest, const char *src, size_t size)
{
    if (size == 0 || dest == NULL) {
        return strlen(src);
    }
    for (size_t i = 0; ; i++) {
        if (i >= size - 1) {
            dest[i] = '\0';
            return i + strlen(src + i);
        }
        if (src[i] == '\0') {
            dest[i] = '\0';
            return i;
        }
        dest[i] = src[i];
    }
}

static struct {
    char c;
    char *esc;
} c2esc[] = { {.c = '\0', .esc = "\\0"}, {.c = '\a', .esc = "\\a"},
              {.c = '\b', .esc = "\\b"}, {.c = '\f', .esc = "\\a"},
              {.c = '\n', .esc = "\\n"}, {.c = '\r', .esc = "\\a"},
              {.c = '\t', .esc = "\\t"}, {.c = '\v', .esc = "\\a"},
              {.c = '\\', .esc = "\\\\"}, {.c = '\"', .esc = "\\\""}
};

size_t cstr_escape(char *dest, const char *src, size_t src_len, size_t size)
{
    long len = 0;
    const char *endptr = src + src_len;
    while (src < endptr) {
        size_t i;
        for (i = 0; i < ARRAY_LEN(c2esc); i++) {
            if (*src == c2esc[i].c) {
                break;
            }
        }
        if (i < ARRAY_LEN(c2esc)) {
            len += cstr_ncopy(dest + len, c2esc[i].esc, max(0, size - len));
        } else if (isprint(*src)) {
            len += cstr_putc(dest + len, *src, max(0, size - len));
        } else {
            len += snprintf(dest + len, max(0, size - len),
                    "\\x%02x", *(unsigned char *)src);
        }
        src++;
    }
    return len;
}

size_t cstr_unescape(char *dest, const char *src, size_t size)
{
    long len = 0;
    while (*src != '\0') {
        if (*src == '\\') {
            size_t i;
            src++;
            for (i = 0; i < ARRAY_LEN(c2esc); i++) {
                if (*src == c2esc[i].esc[1]) {
                    break;
                }
            }
            if (i < ARRAY_LEN(c2esc)) {
                len += cstr_putc(dest + len, c2esc[i].c, max(0, size - len));
            } else if (*src == 'x') {
                char hex[3];
                src++;
                cstr_ncopy(hex, src, sizeof(hex));
                src += sizeof(hex) - 1;
                char c = strtol(hex, NULL, 16);
                len += cstr_putc(dest + len, c, max(0, size - len));
            }
        } else {
            len += cstr_putc(dest + len, *src, max(0, size - len));
        }
        src++;
    }
    return len;
}

bool cstr_is_empty(const char *cstr)
{
    return cstr[0] == '\0';
}

bool cstr_eq(const char *cstr, const char *other)
{
    return strcmp(cstr, other) == 0;
}

size_t cstr_startswith(const char *cstr, const char *start)
{
    size_t pos = 0;
    while(start[pos] != '\0') {
        if (cstr[pos] != start[pos]) {
            pos = 0;
            break;
        }
        pos++;
    }
    return pos;
}

size_t cstr_endswith(const char *cstr, const char *end)
{
    size_t len = 0;
    const char *cstrptr = cstr + strlen(cstr);
    const char *endptr = end + strlen(end);
    while(--endptr >= end) {
        if (--cstrptr < cstr || *cstrptr != *endptr) {
            len = 0;
            break;
        }
        len++;
    }
    return len;
}
