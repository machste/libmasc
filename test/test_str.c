#include <stdio.h>

#include <masc/str.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
    Str *s = new(Str, "Hello %s!", "World");
    put(s);
    printf("lower: %s\n", str_lower(s));
    printf("upper: %s\n", str_upper(s));
    printf("swap:  %s\n", str_swapcase(s));
    // Set other string
    str_copy(s, "   Hallo Welt   ");
    printf("lstrip: '%s'\n", str_lstrip(s));
    str_copy(s, "   Hallo Welt   ");
    printf("rstrip: '%s'\n", str_rstrip(s));
    str_copy(s, "   Hallo Welt   ");
    printf("strip:  '%s'\n", str_strip(s));
    // Iterate over string
    Iter i = str_iter(s);
    for (char *c = next(&i); c != NULL; c = next(&i)) {
        printf("%c\n", *c);
    }
    // Clean-up
    delete(s);
    return 0;
}
