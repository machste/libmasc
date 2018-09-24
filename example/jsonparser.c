#include <masc/argparse.h>
#include <masc/path.h>
#include <masc/file.h>
#include <masc/bool.h>
#include <masc/none.h>
#include <masc/json.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    int ret = 0;
    // Parse command line arguments
    Argparse *ap = new(Argparse, path_basename(argv[0]), "JSON Parser");
    argparse_add_opt(ap, 'c', "compact", NULL, NULL, NULL, "compact output");
    argparse_add_arg(ap, "file", "FILE", NULL, argparse_file, "JSON file");
    argparse_add_arg(ap, "key", "KEY", "?", NULL, "key of a JSON node");
    Map *args = argparse_parse(ap, argc, argv);
    delete(ap);
    File *f = map_remove_key(args, "file");
    Str *key = map_remove_key(args, "key");
    bool compact = bool_get(map_get(args, "compact"));
    delete(args);
    // Read JSON file
    Str *js_str = file_read(f, -1);
    // Parse JSON string
    Json *js = json_new_cstr(str_cstr(js_str));
    if (json_is_valid(js)) {
        if (!is_none(key)) {
            // Get node by key
            void *node = json_get_node(js, str_cstr(key));
            if (node != NULL) {
                if (compact) {
                    put_repr(node);
                } else {
                    pretty_print(node);
                }
            } else {
                fprint(stderr, "Error: No such key '%O'!\n", key);
            }
        } else {
            // Print the whole JSON
            if (compact) {
                put(js);
            } else {
                json_pretty_print(js);
            }
        }
    } else {
        fprint(stderr, "Error: %s!\n", json_err_msg(js));
        ret = -1;
    }
    delete(js);
    delete(js_str);
    delete(key);
    delete(f);
    return ret;
}
