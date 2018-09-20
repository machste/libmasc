#include <masc/regex.h>
#include <masc/bool.h>
#include <masc/macro.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    Regex *re_err = new(Regex, "(Hallo Welt$");
    print("Wrong regex: %O\n", re_err);
    print("nsub: %zu\n", regex_get_nsub(re_err));
    delete(re_err);
    Regex *re = new(Regex, "[0-9]+(\\.[0-9]+)?");
    char *cstrs[] = {"42", "--.23--", "float: 2.34", "A2x5", "A", "V1.2-beta"};
    print("Regex: %O\n", re);
    Bool b = init(Bool, regex_is_match(re, cstrs[1]));
    print("Matches: %s -> %O\n", cstrs[1], &b);
    b = init(Bool, regex_is_match(re, cstrs[4]));
    print("Matches: %s -> %O\n", cstrs[4], &b);
    for (int i = 0; i < ARRAY_LEN(cstrs); i++) {
        Str *match = regex_find(re, cstrs[i]);
        print("cstr: '%s' -> '%O'\n", cstrs[i], match);
        delete(match);
    }
    delete(re);
    re = new(Regex, "([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})");
    print("MAC regex: %O\n", re);
    print("nsub: %zu\n", regex_get_nsub(re));
    char mac[] = "---11:22:33:44:55:66---";
    print("Match MAC: %s\n", mac);
    Array *match = regex_search(re, mac);
    if (match != NULL) {
        put(match);
        delete(match);
    }
    delete(re);
    char *to_be_splitted = "Well, these are some words.";
    re = new(Regex, "\\W+");
    print("Split '%s' by %O\n", to_be_splitted, re);
    List *parts = regex_split(re, to_be_splitted, -1);
    print("parts: %O\n", parts);
    delete(parts);
    delete(re);
}
