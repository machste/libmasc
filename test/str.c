#include <masc/str.h>
#include <masc/char.h>
#include <masc/iter.h>
#include <masc/macro.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
    Str *s = new(Str, "Hello %s?", "World");
    print("s: %O\n", s);
    print("get_at(-3): %c\n", str_get_at(s, -3));
    str_set_at(s, -1, '!');
    print("set_at(s, -1, '!'): %s\n", s->cstr);
    print("swap:  %s\n", str_swapcase(s)->cstr);
    print("lower: %s\n", str_lower(s)->cstr);
    print("upper: %O\n", str_upper(s));
    print("reverse: %O\n", str_reverse(s));
    print("snakecase: %O\n", str_snakecase(s));
    // Escape string und unescape
    str_copy(s, "Hello\nWorld!\tName: \"M\xc3\xa4" "chler\"");
    print("String with special chars:\n%O\n---\n", s);
    print("escape: '%O'\n", str_escape(s));
    // Append invalid escape sequence '\z'
    str_append(s, "\\nBad: '\\z'");
    print("unescape:\n%s\n---\n", str_unescape(s)->cstr);
    // Set other string
    str_copy(s, " Hello World");
    print("ljust:  '%O'\n", str_ljust(s, 20, '*'));
    str_copy(s, "Hello World ");
    print("rjust:  '%O'\n", str_rjust(s, 20, '*'));
    str_copy(s, " Hello World ");
    print("center: '%O'\n", str_center(s, 20, '*'));
    str_copy(s, "   Hello World   ");
    print("lstrip: '%O'\n", str_lstrip(s));
    str_copy(s, "   Hello World   ");
    print("rstrip: '%O'\n", str_rstrip(s));
    str_copy(s, "   Hello World   ");
    print("strip:  '%O'\n", str_strip(s));
    print("startswith(\"He\"): %d\n", str_startswith(s, "He"));
    print("endswith(\"rld\"): %d\n", str_endswith(s, "rdl"));
    print("truncate(9, \"...\"): %O\n", str_truncate(s, 9, "..."));
    str_delete_at(s, 0);
    print("delete_at(s, 0): %s\n", s->cstr);
    str_delete_at(s, -1);
    print("delete_at(s, -1): %s\n", s->cstr);
    str_delete_at(s, 5);
    print("delete_at(s, 5): %s\n", s->cstr);
    str_copy(s, "0123456789");
    print("s: %O, len: %zu\n", s, len(s));
    print("reverse: %O\n", str_reverse(s));
    print("slice(2, 7): '%O'\n", str_slice(s, 2, 7));
    // Iterate over string
    char data[] = "Iterate";
    str_fmt(s, "'%s' has %i chars.", data, sizeof(data));
    print("s: %O\n", s);
    Iter *itr = new(Iter, s);
    for (Char *c = next(itr); c != NULL; c = next(itr)) {
        if (iter_is_last(itr)) {
            print("%O\n", c);
        } else {
            print("%O, ", c);
        }
    }
    delete(itr);
    // Append / Prepend
    Str *split = new(Str, ";;token1;;");
    str_append(split, "token2;;");
    str_append_fmt(split, "token%i;;;;", 3);
    str_prepend_fmt(split, ";token%i", 0);
    // Split string
    char sep[] = ";;";
    print("split: '%O' with '%s'\n", split, sep);
    List *splitted = str_split(split, sep, -1);
    print("splitted: %O\n", splitted);
    // Filter empty strings
    filter(splitted,
            LAMBDA(bool, (Object *s){return !str_is_empty((Str *)s);}));
    print("filtered: %O\n", splitted);
    // Join string again
    Str *joined = str_join(splitted, ", ");
    print("joined: %O\n", joined);
    // Filter all alpha
    filter(joined,
            LAMBDA(bool, (Object *c){return !char_is_alpha((Char *)c);}));
    print("filtered: %O\n", joined);
    // Replace
    Str *replaced = str_replace(joined, ", ", " - ", -1);
    print("replaced: %O\n", replaced);
    // Clean-up
    delete(split);
    delete(splitted);
    delete(joined);
    delete(replaced);
    delete(s);
    return 0;
}
