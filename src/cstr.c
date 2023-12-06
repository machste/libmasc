#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include <masc/cstr.h>
#include <masc/str.h>
#include <masc/math.h>
#include <masc/macro.h>


struct c2esc_t {
    char c;
    char *esc;
};


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
struct c2esc_t c2esc[] = {
    {.c = '\0', .esc = "\\0"}, {.c = '\a', .esc = "\\a"},
    {.c = '\b', .esc = "\\b"}, {.c = '\f', .esc = "\\f"},
    {.c = '\n', .esc = "\\n"}, {.c = '\r', .esc = "\\r"},
    {.c = '\t', .esc = "\\t"}, {.c = '\v', .esc = "\\v"},
    {.c = '\\', .esc = "\\\\"}, {.c = '"', .esc = "\\\""}
};

size_t cstr_escape(char *dest, const char *src, size_t src_len, size_t size)
{
    long len = 0;
    const char *endptr = src + src_len;
    while (src < endptr) {
        // If character is printable use it, ...
        if (isprint(*src)) {
            len += cstr_putc(dest + len, *src++, max(0, size - len));
            continue;
        }
        // ... otherwise escape the character.
        size_t i;
        for (i = 0; i < ARRAY_LEN(c2esc); i++) {
            if (*src == c2esc[i].c) {
                break;
            }
        }
        if (i < ARRAY_LEN(c2esc)) {
            len += cstr_ncopy(dest + len, c2esc[i].esc, max(0, size - len));
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
                src += sizeof(hex) - 2;
                char c = strtol(hex, NULL, 16);
                len += cstr_putc(dest + len, c, max(0, size - len));
            } else {
                // Invalid escape sequences will be ignored.
            }
        } else {
            len += cstr_putc(dest + len, *src, max(0, size - len));
        }
        src++;
    }
    return len;
}

struct c2esc_t c2jesc[] = {
    {.c = '"', .esc = "\\\""}, {.c = '\\', .esc = "\\\\"},
    {.c = '\b', .esc = "\\b"}, {.c = '\f', .esc = "\\f"},
    {.c = '\n', .esc = "\\n"}, {.c = '\r', .esc = "\\r"},
    {.c = '\t', .esc = "\\t"}
};

static uint8_t _check_utf8_seq(const char *cstr)
{
    uint8_t first_byte = (uint8_t)*cstr;
    if (first_byte < 0x80) {
        // Normal ASCII character
        return 1;
    }
    uint8_t len = 0;
    if (first_byte & 0xC0) {
        len = 1;
        while (first_byte & 0x40) {
            cstr++;
            if (((uint8_t)*cstr & 0xc0) != 0x80) {
                // Invalid continuation of UTF-8 sequence!
                return 0;
            }
            first_byte <<= 1;
            len++;
        }
    }
    return len;
}

size_t cstr_repr(char *dest, const char *src, size_t src_len, size_t size)
{
    long len = 0;
    const char *endptr = src + src_len;
    len = cstr_putc(dest, '"', size);
    while (src < endptr) {
        // Check for well-known escape sequence or ...
        const char *esc = NULL;
        for (size_t i = 0; i < ARRAY_LEN(c2jesc); i++) {
            if (*src == c2jesc[i].c) {
                esc = c2jesc[i].esc;
                break;
            }
        }
        if (esc != NULL) {
            len += cstr_ncopy(dest + len, esc, max(0, size - len));
            src++;
            continue;
        }
        // ... check for printable character or ...
        if (isprint(*src)) {
            len += cstr_putc(dest + len, *src++, max(0, size - len));
            continue;
        }
        // ... check for UTF-8 sequence.
        uint8_t utf8_len = _check_utf8_seq(src);
        if (utf8_len > 1) {
            // Copy UTF-8 sequence
            for (size_t i = 0; i < utf8_len; i++) {
                len += cstr_putc(dest + len, *src++, max(0, size - len));
            }
            continue;
        }
        // If nothing applies escape single byte with '\x'.
        len += snprintf(dest + len, max(0, size - len),
                "\\x%02x", *(unsigned char *)src++);
    }
    len += cstr_putc(dest + len, '"', max(0, size - len));
    return len;
}

size_t cstr_unrepr(char *dest, const char *src, size_t size){
    long len = 0;
    while (*src != '\0') {
        if (*src == '"') {
            // Skip all '"' characters
        } else if (*src == '\\') {
            size_t i;
            src++;
            for (i = 0; i < ARRAY_LEN(c2jesc); i++) {
                if (*src == c2jesc[i].esc[1]) {
                    break;
                }
            }
            if (i < ARRAY_LEN(c2jesc)) {
                len += cstr_putc(dest + len, c2jesc[i].c, max(0, size - len));
            } else if (*src == 'u') {
                char hex[5];
                src++;
                cstr_ncopy(hex, src, sizeof(hex));
                src += sizeof(hex) - 2;
                uint16_t u = strtol(hex, NULL, 16);
                if (u < 0x80) {
                    len += cstr_putc(dest + len, u, max(0, size - len));
                } else if (u < 0x800) {
                    uint8_t b1 = 0xc0 | (u >> 6);
                    len += cstr_putc(dest + len, b1, max(0, size - len));
                    uint8_t b2 = 0x80 | (u & 0x03f);
                    len += cstr_putc(dest + len, b2, max(0, size - len));
                } else {
                    uint8_t b1 = 0xe0 | (u >> 12);
                    len += cstr_putc(dest + len, b1, max(0, size - len));
                    uint8_t b2 = 0x80 | ((u >> 6) & 0x03f);
                    len += cstr_putc(dest + len, b2, max(0, size - len));
                    uint8_t b3 = 0x80 | (u & 0x03f);
                    len += cstr_putc(dest + len, b3, max(0, size - len));
                }
            } else if (*src == 'x') {
                char hex[3];
                src++;
                cstr_ncopy(hex, src, sizeof(hex));
                src += sizeof(hex) - 2;
                char c = strtol(hex, NULL, 16);
                len += cstr_putc(dest + len, c, max(0, size - len));

            } else {
                // Invalid escape sequences will be ignored.
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

bool cstr_is_utf8(const char *cstr)
{
    while (*cstr != '\0') {
        unsigned char utf8_len = _check_utf8_seq(cstr);
        if (utf8_len == 0) {
            return false;
        }
        cstr += utf8_len;
    }
    return true;
}

bool cstr_eq(const char *cstr, const char *other)
{
    return strcmp(cstr, other) == 0;
}

const char *cstr_skip_spaces(const char *cstr)
{
    while(*cstr != '\0' && isspace(*cstr)) {
        cstr++;
    }
    return cstr;
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

char *cstr_lower(char *cstr)
{
    for (char *c = cstr; *c != '\0'; c++) {
        *c = tolower(*c);
    }
    return cstr;
}

char *cstr_upper(char *cstr)
{
    for (char *c = cstr; *c != '\0'; c++) {
        *c = toupper(*c);
    }
    return cstr;
}

char *cstr_swapcase(char *cstr)
{
    for (char *c = cstr; *c != '\0'; c++) {
        if (islower(*c)) {
            *c = toupper(*c);
        } else if (isupper(*c)) {
            *c = tolower(*c);
        }
    }
    return cstr;
}

List *cstr_split(const char *cstr, const char *sep, int maxsplit)
{
    List *l = list_new();
    const char *start = cstr, *end = cstr;
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

static long _strtol(const char *nptr, char **endptr)
{
    long value = 0;
    const char *cstr = cstr_skip_spaces(nptr);
    if (cstr[0] == '0' && (cstr[1] == 'b' || cstr[1] == 'B')) {
        value = strtol(cstr + 2, endptr, 2);
        if (*endptr == cstr + 2) {
            *endptr = (char *)cstr + 1;
        }
    } else {
        value = strtol(nptr, endptr, 0);
    }
    return value;
}

Int *cstr_to_int(const char *cstr, bool strict, char **endptr)
{
    Int *i = NULL;
    char *_dummy_endptr;
    if (endptr == NULL) { endptr = &_dummy_endptr; }
    long value = _strtol(cstr, endptr);
    if (*endptr != cstr && (!strict || (strict && **endptr == '\0'))) {
        i = int_new(value);
    }
    return i;
}

Double *cstr_to_double(const char *cstr, bool strict, char **endptr)
{
    Double *d = NULL;
    char *_dummy_endptr;
    if (endptr == NULL) { endptr = &_dummy_endptr; }
    double value = strtod(cstr, endptr);
    if (*endptr != cstr && (!strict || (strict && **endptr == '\0'))) {
        d = double_new(value);
    }
    return d;
}

Num *cstr_to_number(const char *cstr, bool strict, char **endptr)
{
    Num *num_obj = NULL;
    char *dend, *lend;
    double d = strtod(cstr, &dend);
    long l = _strtol(cstr, &lend);
    if (endptr != NULL) { *endptr = (char *)cstr; }
    if (dend != cstr) {
        if (dend <= lend && (!strict || (strict && *lend == '\0'))) {
            num_obj = int_new(l);
            if (endptr != NULL) { *endptr = lend; }
        } else if (!strict || (strict && *dend == '\0')) {
            num_obj = double_new(d);
            if (endptr != NULL) { *endptr = dend; }
        }
    }
    return num_obj;
}
