#include <masc/file.h>
#include <masc/json.h>
#include <masc/print.h>


static void print_usage(void)
{
    fprint(stderr, "usage: jsonparser FILE [KEY]\n");
}

int main(int argc, char *argv[])
{
    int ret = 0;
    char *key = NULL;
    if (argc == 3) {
        key = argv[2];
    } else if (argc < 2 || argc > 3) {
        print_usage();
        return -1;
    }
    File *f = new(File, argv[1], "r");
    if (file_is_open(f)) {
        Str *js_str = file_read(f, -1);
        Json *js = json_new_cstr(str_cstr(js_str));
        if (json_is_valid(js)) {
            if (key != NULL) {
                void *node = json_get_node(js, key);
                if (node != NULL) {
                    put(node);
                } else {
                    fprint(stderr, "Error: No such key '%s'!\n", key);
                }
            } else {
                json_pretty_print(js);
            }
        } else {
            fprint(stderr, "Error: %s!\n", json_err_msg(js));
            ret = -1;
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
