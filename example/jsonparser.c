#include <masc/file.h>
#include <masc/json.h>
#include <masc/print.h>


static void print_usage(void)
{
    fprint(stderr, "usage: jsonparser FILE\n");
}

int main(int argc, char *argv[])
{
    int ret = 0;
    if (argc != 2) {
        print_usage();
        return -1;
    }
    File *f = new(File, argv[1], "r");
    if (file_is_open(f)) {
        Str *js_str = file_read(f, -1);
        Json *js = json_new_cstr(str_cstr(js_str));
        if (json_is_valid(js)) {
            json_pretty_print(js);
        } else {
            fprint(stderr, "Error: %s!\n", json_err_msg(js));
        }
        delete(js);
        delete(js_str);       
    } else {
        fprint(stderr, "Error: file '%s': %s!\n", file_path(f),
                file_err_msg(f));
        ret = -1;
    }
    delete(f);
    return ret;
}
