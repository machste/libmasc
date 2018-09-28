#include <masc/argparse.h>
#include <masc/path.h>
#include <masc/file.h>
#include <masc/bool.h>
#include <masc/none.h>
#include <masc/json.h>
#include <masc/print.h>


static void *value_check(Str *value, Str **err_msg)
{
    Json *js_val = new(Json, NULL);
    JsonError err = json_parse(js_val, str_cstr(value));
    if (err == JSON_SUCCESS) {
        return json_get_root(js_val);
    } else {
        void *val = NULL;
        if (str_is_match(value, "[^\\{\\}\\[\\]]*")) {
            val = new_copy(value);
        } else {
            *err_msg = str_new("invalid value: '%O' (%s)!", value,
                    json_err_msg(js_val));
        }
        delete(js_val);
        return val;
    }
}

static int get_action(Json *js, Str *key, bool compact)
{
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
            return -1;
        }
    } else {
        // Print the whole JSON
        if (compact) {
            put(js);
        } else {
            json_pretty_print(js);
        }
    }
    return 0;
}

static int set_action(Json *js, Str *path, Str *key, void *value, bool compact)
{
    int ret = 0;
    if (json_set_node(js, str_cstr(key), new_copy(value))) {
        // Write new json content to json file
        File *js_file = new(File, str_cstr(path), "w");
        if (file_is_open(js_file)) {
            Str *js_str;
            if (compact) {
                js_str = to_str(js);
            } else {
                js_str = json_pretty_str(js);
            }
            file_write(js_file, str_cstr(js_str));
            file_write(js_file, "\n");
            delete(js_str);
        } else {
            fprint(stderr, "Error: File '%s': %s!\n", file_path(js_file),
                    file_err_msg(js_file));
            ret = -1;
        }
        delete(js_file);
    } else {
        fprint(stderr, "Error: Unable to set key '%O'!\n", key);
        ret = -1;
    }
    return ret;
}


int main(int argc, char *argv[])
{
    int ret = 0;
    // Setup argument parser
    Argparse *ap = new(Argparse, path_basename(argv[0]), "JSON Parser");
    argparse_add_opt(ap, 'c', "compact", NULL, NULL, NULL, "compact output");
    argparse_add_arg(ap, "path", "FILE", NULL, NULL, "JSON file");
    argparse_add_arg(ap, "key", "KEY", "?", NULL, "key of JSON node");
    argparse_add_arg(ap, "value", "VALUE", "?", value_check,
            "value of JSON node");
    // Parse command line arguments
    Map *args = argparse_parse(ap, argc, argv);
    delete(ap);
    bool compact = bool_get(map_get(args, "compact"));
    Str *key = map_remove_key(args, "key");
    void *value = map_remove_key(args, "value");
    Str *path = map_remove_key(args, "path");
    delete(args);
    // Open JSON file, read and parse it
    Json *js = NULL;
    File *js_file = new(File, str_cstr(path), "r");
    if (file_is_open(js_file)) {
        Str *js_str = file_read(js_file, -1);
        js = json_new_cstr(str_cstr(js_str));
        delete(js_str);
        if (!json_is_valid(js)) {
            fprint(stderr, "Error: %s!\n", json_err_msg(js));
            ret = -1;
        }
    } else {
        fprint(stderr, "Error: File '%s': %s!\n", file_path(js_file),
                file_err_msg(js_file));
        ret = -1;
    }
    delete(js_file);
    // Perform get or set action
    if (ret == 0) {
        if (is_none(value)) {
            ret = get_action(js, key, compact);
        } else {
            ret = set_action(js, path, key, value, compact);
        }
    }
    if (js != NULL) {
        delete(js);
    }
    delete_objs(key, value, path);
    return ret;
}
