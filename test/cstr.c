#include <string.h>

#include <masc/cstr.h>
#include <masc/print.h>
#include <masc/utils.h>


int main(int argc, char *argv[])
{
    const char *cstr = cstr_skip_spaces(" \t\n Hi ,\n\t -> Steve!");
    print("> cstr_skip_spaces(\" \\t\\n Hi ,\\n\\t -> Steve!\");\n%s\n", cstr);
    Int *i = cstr_to_int("0b10101010", true, NULL);
    print("'0b10101010' -> %O\n", i);
    delete(i);
    char *endptr;
    i = cstr_to_int("  0b11001gugus", false, &endptr);
    print("'  0b11001gugus' -> %O (%c)\n", i, *endptr);
    delete(i);
    Num *n = cstr_to_number("0xab", true, NULL);
    print("'0xab' -> %O\n", n);
    delete(n);
    n = cstr_to_number("0b1001", true, NULL);
    print("'0b1001' -> %O\n", n);
    delete(n);
    const char *utf8 = "2H\xe2\x82\x82 + O\xe2\x82\x82 \xe2\x87\x8c " \
            "2H\xe2\x82\x82O,\tR = 4.7 k\xce\xa9,\t\xe2\x8c\x80 200 mm,\t" \
            "\xf0\x9d\x84\x9e \xf0\x9d\x85\xa0";
    bool utf8_ok = cstr_is_utf8(utf8);
    print("UTF-8: '%s' -> %s\n", utf8, utf8_ok ? "ok" : "err");
    char s[256];
    cstr_repr(s, utf8, strlen(utf8), sizeof(s));
    print("repr: %s\n", s);
    utf8 = "Hallo Stefan Mächler!\n\tBad UTF-8: \"\xf0\x9d\x05\xa0\"\n\t\\u20ac";
    utf8_ok = cstr_is_utf8(utf8);
    print("UTF-8: '%s' -> %s\n", utf8, utf8_ok ? "ok" : "err");
    cstr_repr(s, utf8, strlen(utf8), sizeof(s));
    print("repr: %s\n", s);
    char unrepr_s[256];
    memset(unrepr_s, 0, sizeof(unrepr_s));
    cstr_unrepr(unrepr_s, s, sizeof(unrepr_s));
    print("unrepr:\n");
    hexdump(unrepr_s, (strlen(unrepr_s) / 16 + 1) * 16);
    print("UTF-8: '%s'\n", unrepr_s);
    return 0;
}
