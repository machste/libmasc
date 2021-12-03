#include <masc/cstr.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    const char *cstr = cstr_skip_spaces(" \t\n Hi ,\n\t -> Steve!");
    print("> cstr_skip_spaces(\" \\t\\n Hi ,\\n\\t -> Steve!\");\n%s\n", cstr);
    return 0;
}
