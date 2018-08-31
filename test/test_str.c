#include <masc/str.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
    Str *s = new(Str, "Hello %s!", "World");
    print("s: %O\n", s);
    print("lower: %s\n", str_lower(s)->cstr);
    print("upper: %O\n", str_upper(s));
    print("swap:  %O\n", str_swapcase(s));
    // Set other string
    str_copy(s, "   Hallo Welt   ");
    print("lstrip: '%O'\n", str_lstrip(s));
    str_copy(s, "   Hallo Welt   ");
    print("rstrip: '%O'\n", str_rstrip(s));
    str_copy(s, "   Hallo Welt   ");
    print("strip:  '%O'\n", str_strip(s));
    print("slice(2, 7): '%O'\n", str_slice(s, 2, 7));
    // Iterate over string
    char data[] = "Interate";
    str_fmt(s, "'%s' has %i chars.", data, sizeof(data));
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
    delete(s);
    return 0;
}
