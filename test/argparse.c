#include <masc/argparse.h>
#include <masc/path.h>
#include <masc/macro.h>
#include <masc/print.h>


void *cmd_check(Str *cmd, Str **err_msg)
{
    return new_copy(cmd);
}

int main(int argc, char *argv[])
{
    Argparse *ap = new(Argparse, path_basename(argv[0]),
            "This is the help text of the argparse test program.");
    argparse_add_opt(ap, 't', NULL, NULL, NULL, NULL, "help for flag t");
    argparse_add_opt(ap, 'f', "foo", NULL, "2", argparse_num, "help for foo");
    argparse_add_arg(ap, "cmd", "CMD", NULL, cmd_check, "help test for num");
    argparse_add_arg(ap, "file", "FILE", "+", argparse_file, NULL);
    Map *args;
    if(argc == 1) {
        // Test program
        put(ap);
        argparse_print_help(ap);
        char *test_args[] = {"", "-f", "12", "0.5", "set", "-t", "/etc/passwd"};
        args = argparse_parse(ap, ARRAY_LEN(test_args), test_args);
    } else {
        // Manual tests
        args = argparse_parse(ap, argc, argv);
    }
    put(args);
    delete(args);
    delete(ap);
    return 0;
}
