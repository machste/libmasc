#include <masc.h>


static int rm_lines(Regex *re, File *f, bool invert)
{
    int ret = 0;
    int rm_line_count = 0;
    List *lines = file_readlines(f);
    // Define filter callback function
    bool re_match(Str *line) {
        bool keep_line = regex_is_match(re, str_cstr(line)) == invert;
        if (!keep_line) {
            rm_line_count++;
        }
        return keep_line;
    }
    // Filter the lines of the file
    filter(lines, (filter_cb)re_match);
    // Write the remaining lines
    if (file_reopen(f, "w")) {
        file_writelines(f, lines);
        print("Removed %i lines from '%s'\n", rm_line_count, file_path(f));
    } else {
        fprint(stderr, "File '%s': %s!\n", file_path(f), file_err_msg(f));
        ret = -1;
    }
    delete(lines);
    return ret;
}

static void *check_regex(Str *regex, Str **err_msg)
{
    Regex *re = new(Regex, str_cstr(regex));
    if (!regex_is_valid(re)) {
        *err_msg = new(Str, "invalid regex: %O!", re);
        delete(re);
        re = NULL;
    }
    return re;
} 

int main(int argc, char *argv[])
{
    // Setup argument parser
    Argparse *ap = new(Argparse, path_basename(argv[0]),
            "Remove lines from a file which match a given regular expression.");
    argparse_add_opt(ap, 'i', "invert", NULL, NULL, NULL, "invert regex match");
    argparse_add_arg(ap, "file", "FILE", NULL, argparse_file,
            "file to remove lines");
    argparse_add_arg(ap, "regex", "REGEX", NULL, check_regex,
            "regular expression");
    // Parse command line arguments
    Map *args = argparse_parse(ap, argc, argv);
    delete(ap);
    bool invert = bool_get(map_get(args, "invert"));
    File *f = map_get(args, "file");
    Regex *re = map_get(args, "regex");
    // Remove lines
    int ret = rm_lines(re, f, invert);
    // Cleanup
    delete(args);
    return ret;
}
