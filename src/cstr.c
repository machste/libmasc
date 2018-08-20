#include <string.h>

#include <masc/cstr.h>


size_t cstr_copy(char *dest, const char *src, size_t size)
{
    if (size == 0 || dest == NULL) {
        return strlen(src);
    }
    for (size_t i = 0; ; i++) {
        if (i > size - 1) {
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
