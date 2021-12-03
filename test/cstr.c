#include <masc/cstr.h>
#include <masc/print.h>


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
    return 0;
}
