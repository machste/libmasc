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
    Map *section = NULL;
    Regex *re_cfg = new(Regex, "^config\\s+(\\w+)(\\s+'(\\w+)')?");
    Regex *re_opt = new(Regex, "^\\s*option\\s+(\\w+)\\s+'(\\w+)'");
    // Add sections map to uci map use the file name
    map_set(uci, file_basename(uci_file), sections);
    // Parse uci file line by line
    Str *line;
    for (int i = 0; (line = file_readline(uci_file)) != NULL; i++) {
        Array *match = NULL;
        if ((match = regex_search(re_cfg, str_cstr(line))) != NULL) {
            // Found new section definition
            Str *type = array_get_at(match, 1);
            Str *name = array_get_at(match, 3);
            if (is_none(name)) {
                // Handle anonymous section (@type)
                name = new(Str, "@%O", type);
                List *anon_list = map_get(sections, str_cstr(name));
                if (anon_list == NULL) {
                    anon_list = new(List);
                    map_set(sections, str_cstr(name), anon_list);
                }
                section = new(Map);
                map_set(section, "__sec_type__", new_copy(type));
                list_append(anon_list, section);
                delete(name);
            } else {
                // Handle named section
                section = new(Map);
                map_set(section, "__sec_type__", new_copy(type));
                map_set(sections, str_cstr(name), section);
            }
        } else if ((match = regex_search(re_opt, str_cstr(line))) != NULL) {
            // Add option to current section
            if (section != NULL) {
                Str *name = array_get_at(match, 1);
                Str *value = array_get_at(match, 2);
                // Try to convert value to number
                void *num = str_to_number(value, true);
                if (num != NULL) {
                    map_set(section, str_cstr(name), num);
                } else {
                    map_set(section, str_cstr(name), new_copy(value));
                }
            }
        }
        delete_objs(match, line);
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
