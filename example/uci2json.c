#include <masc/file.h>
#include <masc/map.h>
#include <masc/regex.h>
#include <masc/none.h>
#include <masc/json.h>
#include <masc/print.h>


static void print_usage(void)
{
    fprint(stderr, "usage: uci2json FILE\n");
}

static Map *uci_parse(File *uci_file)
{
    Map *uci = new(Map);
    Map *sections = new(Map);
    Map *options = NULL;
    Regex *re_cfg = new(Regex, "^config\\s+(\\w+)(\\s+'(\\w+)')?");
    Regex *re_opt = new(Regex, "^\\s*option\\s+(\\w+)\\s+'(\\w+)'");
    // Add sections map to uci map use the file name
    map_set(uci, file_basename(uci_file), sections);
    // Parse uci file line by line
    Str *line;
    for (int i = 0; (line = file_readline(uci_file)) != NULL; i++) {
        Array *match = NULL;
        if ((match = regex_search(re_cfg, str_cstr(line))) != NULL) {
            Str *type = array_get_at(match, 1);
            Str *name = array_get_at(match, 3);
            if (is_none(name)) {
                name = new(Str, "@%O", type);
                List *anon_list = map_get(sections, str_cstr(name));
                if (anon_list == NULL) {
                    anon_list = new(List);
                    map_set(sections, str_cstr(name), anon_list);
                }
                Map *sec = new(Map);
                map_set(sec, "type", new_copy(type));
                options = new(Map);
                map_set(sec, "options", options);
                list_append(anon_list, sec);
                delete(name);
            } else {
                Map *sec = new(Map);
                map_set(sec, "type", new_copy(type));
                options = new(Map);
                map_set(sec, "options", options);
                map_set(sections, str_cstr(name), sec);
            }
        } else if ((match = regex_search(re_opt, str_cstr(line))) != NULL) {
            if (options != NULL) {
                Str *name = array_get_at(match, 1);
                Str *value = array_get_at(match, 2);
                map_set(options, str_cstr(name), new_copy(value));
            }
        }
        delete(match);
        delete(line);
    }
    return uci;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    if (argc != 2) {
        print_usage();
        return -1;
    }
    File *uci_file = new(File, argv[1], "r");
    if (file_is_open(uci_file)) {
        Map *uci = uci_parse(uci_file);
        Json *js = new(Json, uci);
        delete(uci);
        json_pretty_print(js);
        delete(js);
    } else {
        fprint(stderr, "Error: file '%s': %s!\n", file_path(uci_file),
                file_err_msg(uci_file));
        ret = -1;
    }
    delete(uci_file);
    return ret;
}
