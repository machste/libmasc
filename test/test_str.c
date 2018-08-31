#include <masc/str.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
    Str *s = new(Str, "Hello %s!", "World");
    print("s: %O\n", s);
    print("lower: %s\n", str_lower(s));
    print("upper: %s\n", str_upper(s));
    print("swap:  %s\n", str_swapcase(s));
    // Set other string
    str_copy(s, "   Hallo Welt   ");
    print("lstrip: '%s'\n", str_lstrip(s));
    str_copy(s, "   Hallo Welt   ");
    print("rstrip: '%s'\n", str_rstrip(s));
    str_copy(s, "   Hallo Welt   ");
    print("strip:  '%s'\n", str_strip(s));
    Str *slice = str_slice(s, 2, 7);
    print("slice(2, 7): '%O'\n", slice);
    // Iterate over string
    Iter i = str_iter(s);
    for (char *c = next(&i); c != NULL; c = next(&i)) {
        if (is_last(&i)) {
            print("%c\n", *c);
        } else {
            print("%c, ", *c);
        }
    }
    // Append
    Str *split = new(Str, ";;token1;;");
    str_append(split, "token2;;");
    str_append_fmt(split, "token%i;;;;", 3);
    print("split: %O\n", split);
    // Split string
    List *splitted = str_split(split, ";;", -1);
    put(splitted);
    // Clean-up
    delete(split);
    delete(splitted);
    delete(slice);
    delete(s);
    return 0;
}
