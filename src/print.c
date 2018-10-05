#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <masc/print.h>
#include <masc/map.h>
#include <masc/list.h>
#include <masc/iter.h>
#include <masc/str.h>
#include <masc/cstr.h>
#include <masc/math.h>


void put(const void *obj)
{
    Str *s = to_str(obj);
    puts(str_cstr(s));
    delete(s);
}

void put_repr(const void *obj)
{
    size_t size = repr(obj, NULL, 0) + 1;
    char *cstr = malloc(size);
    repr(obj, cstr, size);
    puts(cstr);
    free(cstr);
}

size_t print(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    size_t len = vfprint(stdout, fmt, va);
    va_end(va);
    return len;
}

size_t fprint(FILE *stream, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    size_t len = vfprint(stream, fmt, va);
    va_end(va);
    return len;
}

size_t vfprint(FILE *stream, const char *fmt, va_list va)
{
    int fd = fileno(stream);
    if (fd >= 0) {
        return vdprint(fd, fmt, va);
    }
    return 0;
}

size_t dprint(int fd, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    size_t len = vdprint(fd, fmt, va);
    va_end(va);
    return len;
}

size_t vdprint(int fd, const char *fmt, va_list va)
{
    va_list va2;
    // Make a copy of va to use it twice
    va_copy(va2, va);
    size_t len = vformat(NULL, 0, fmt, va);
    char *cstr = malloc(len + 1);
    vformat(cstr, len + 1, fmt, va2);
    write(fd, cstr, len);
    free(cstr);
    va_end(va2);
    return len;
}

size_t format(char *cstr, size_t size, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    size_t len = vformat(cstr, size, fmt, va);
    va_end(va);
    return len;
}

size_t vformat(char *cstr, size_t size, const char *fmt, va_list va)
{
    long len = 0;
    char const *p = fmt, *end = p + strlen(fmt), *fmt_p = fmt, *fmt_q = fmt;
    va_list ap_p, ap_q;
    va_copy(ap_p, va);
    va_copy(ap_q, va);
    // Parse the fmt string by looping over each singel char
    do {
        // Check for literal char
        if ((*p != '%') || (*++p == '%')) {
            fmt_q = p + 1;
            continue;
        }
        // Check for parameter field
        char *q;
        strtoul(p, &q, 10);
        if ((q != p) && (*q == '$')) {
            p = q + 1;
        }
        // Check for flags
        bool flags_quit = false;
        do {
            switch (*p) {
            case '-':
            case '+':
            case ' ':
            case '0':
            case '#':
                p++;
                break;
            default:
                flags_quit = true;
                break;
            }
        } while (!flags_quit || p >= end);
        // Check for width field
        if (*p == '*') {
            va_arg(ap_q, int);
            p++;
        } else {
            strtoul(p, &q, 10);
            p = q;
        }
        // Check for precision field
        if (*p == '.') {
            p++;
            strtoul(p, &q, 10);
            p = q;
        }
        // Length modifiers
        char length[2] = { '\0', '\0' };
        switch (*p) {
        case 'h':
        case 'l':
            length[0] = *p++;
            if ((*p == 'h') || (*p == 'l')) length[1] = *p++;
            break;
        case 'L':
        case 'z':
        case 'j':
        case 't':
            length[0] = *p++;
            break;
        }
        // Types
        switch (*p) {
        // int or unsigned int
        case 'i':
        case 'd':
        case 'u':
        case 'x':
        case 'X':
        case 'o':
            switch (length[0]) {
            case '\0':
                // no length modifier
                if ((*p == 'i') || (*p == 'd')) {
                    // int
                    va_arg(ap_q, int);
                } else {
                    // unsigned int
                    va_arg(ap_q, unsigned int);
                }
                break;
            case 'h':
                if (length[1] == 'h') {
                    // char (promoted to int)
                    va_arg(ap_q, int);
                } else {
                    // short (promoted to int)
                    va_arg(ap_q, int);
                }
                break;
            case 'L':
                if ((*p == 'i') || (*p == 'd')) {
                    if (length[1] == 'L') {
                        // long
                        va_arg(ap_q, long);
                    } else {
                        // long long
                        va_arg(ap_q, long long);
                    }
                } else {
                    if (length[1] == 'L') {
                        // unsigned long
                        va_arg(ap_q, unsigned long);
                    } else {
                        // unsigned long long
                        va_arg(ap_q, unsigned long long);
                    }
                }
                break;
            case 'z':
                // size_t
                va_arg(ap_q, size_t);
                break;
            case 'j':
                // intmax_t
                va_arg(ap_q, intmax_t);
                break;
            case 't':
                // ptrdiff_t
                va_arg(ap_q, ptrdiff_t);
                break;
            }
            break;
        // double
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
        case 'a':
        case 'A':
            switch (length[0]) {
            case 'L':
                // long double
                va_arg(ap_q, long double);
                break;
            case 'l':
                // Does nothing
            default:
                // double
                va_arg(ap_q, double);
            }
            break;
        case 's':
            // char *
            va_arg(ap_q, char *);
            break;
        case 'c':
            // char (promoted to int)
            va_arg(ap_q, int);
            break;
        case 'p':
            // void *
            va_arg(ap_q, void *);
            break;
            // int *
        case 'n':
            va_arg(ap_q, int *);
            break;
        // Custom type
        case 'O': {
            void *obj = va_arg(ap_q, void *);
            if (fmt_q != fmt_p) {
                char *sub_fmt = strndup(fmt_p, fmt_q - fmt_p);
                len += vsnprintf(cstr + len, max(0, size - len), sub_fmt, ap_p);
                free(sub_fmt);
                // 'ap_p' can only be used once, end it.
                va_end(ap_p);
            }
            len += to_cstr(obj, cstr + len, max(0, size - len));
            // Initialise 'ap_p' with the next vas
            va_copy(ap_p, ap_q);
            fmt_p = p + 1;
            break;
        }
        default:
            break;
        }
        fmt_q = p + 1;
    } while (++p < end);
    //Print out the rest of the format string.
    if (*fmt_p) {
        len += vsnprintf(cstr + len, max(0, size - len), fmt_p, ap_p);
    }
    // Cleanup
    va_end(ap_p);
    va_end(ap_q);
    return len;
}

static size_t indent_cstr(int level, char *cstr, size_t size)
{
    int len = level * 2;
    if (level > 0 && size > len) {
        memset(cstr, ' ', len);
    }
    return len;
}

static size_t obj_pretty_cstr(const void *obj, bool last, int level, char *cstr,
        size_t size)
{
    long len = 0;
    if (isinstance(obj, Map)) {
        len += cstr_ncopy(cstr + len, "{\n", max(0, size - len));
        level++;
        Iter *itr = new(Iter, obj);
        for (void *o = next(itr); o != NULL; o = next(itr)) {
            len += indent_cstr(level, cstr + len, max(0, size - len));
            len += snprintf(cstr + len, max(0, size - len), "\"%s\": ",
                    iter_get_key(itr));
            len += obj_pretty_cstr(o, iter_is_last(itr), level, cstr + len,
                    max(0, size - len));
        }
        if (last) {
            len += indent_cstr(--level, cstr + len, max(0, size - len));
            len += cstr_ncopy(cstr + len, "}\n", max(0, size - len));
        } else {
            len += indent_cstr(--level, cstr + len, max(0, size - len));
            len += cstr_ncopy(cstr + len, "},\n", max(0, size - len));

        }
        delete(itr);
    } else if (isinstance(obj, List)) {
        len += cstr_ncopy(cstr + len, "[\n", max(0, size - len));
        level++;
        Iter *itr = new(Iter, obj);
        for (void *o = next(itr); o != NULL; o = next(itr)) {
            len += indent_cstr(level, cstr + len, max(0, size - len));
            len += obj_pretty_cstr(o, iter_is_last(itr), level, cstr + len,
                    max(0, size - len));
        }
        if (last) {
            len += indent_cstr(--level, cstr + len, max(0, size - len));
            len += cstr_ncopy(cstr + len, "]\n", max(0, size - len));
        } else {
            len += indent_cstr(--level, cstr + len, max(0, size - len));
            len += cstr_ncopy(cstr + len, "],\n", max(0, size - len));
        }
        delete(itr);
    } else {
        len += repr(obj, cstr + len, max(0, size - len));
        if (last) {
            len += cstr_ncopy(cstr + len, "\n", max(0, size - len));
        } else {
            len += cstr_ncopy(cstr + len, ",\n", max(0, size - len));
        }
    }
    if (level == 0) {
        // Remove trailing newline
        len--;
        if (max(0, size - len) > 0) {
            cstr[len] = '\0';
        }
    }
    return len;
}

size_t pretty_cstr(const void *obj, char *cstr, size_t size)
{
    return obj_pretty_cstr(obj, true, 0, cstr, size);
}

size_t pretty_print(const void *obj)
{
    size_t size = pretty_cstr(obj, NULL, 0) + 1;
    char *cstr = malloc(size);
    pretty_cstr(obj, cstr, size);
    puts(cstr);
    free(cstr);
    return size - 1;
}
