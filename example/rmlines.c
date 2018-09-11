#include <masc/file.h>
#include <masc/list.h>
#include <masc/regex.h>
#include <masc/print.h>


static void print_usage(void)
{
    fprint(stderr, "usage: rmlines REGEX FILE\n");
}

static int rm_lines(Regex *re, File *f)
{
    int ret = 0;
    int rm_line_count = 0;
    List *lines = file_readlines(f);
    bool re_match(Str *line) {
        bool keep_line = !regex_is_match(re, str_cstr(line));
        if (!keep_line) {
            rm_line_count++;
        }
        return keep_line;
    }
    filter(lines, (filter_cb)re_match);
    if (file_reopen(f, "w")) {
        file_writelines(f, lines);
        print("Removed %i lines from '%s'\n", rm_line_count, file_path(f));
    } else {
        fprint(stderr, "Invalid regex: %O\n", re);
        ret = -1;
    }
    delete(lines);
    return ret;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    if (argc != 3) {
        print_usage();
        return -1;
    }
    Regex *re = new(Regex, argv[1]);
    if (regex_is_valid(re)) {
        File *f = new(File, argv[2], "r+");
        if (file_is_open(f)) {
            ret = rm_lines(re, f);
        } else {
            fprint(stderr, "File '%s': %s!\n", file_path(f), file_err_msg(f));
            ret = -1;
        }
        delete(f);
    } else {
        fprint(stderr, "Invalid regex: %O\n", re);
        ret = -1;
    }
    delete(re);
    return ret;
}
