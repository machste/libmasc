#include <masc/file.h>
#include <masc/list.h>
#include <masc/map.h>
#include <masc/num.h>
#include <masc/json.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    List *users = new(List);
    File *f = new(File, "/etc/passwd", "r");
    if (file_is_open(f)) {
        Str *line;
        while ((line = file_readline(f)) != NULL) {
            str_rstrip(line);
            List *user_data = str_split(line, ":", -1);
            if (list_len(user_data) == 7) {
                Map *user = new(Map);
                map_set(user, "Username", list_remove_at(user_data, 0));
                map_set(user, "Password", list_remove_at(user_data, 0));
                Str *s = list_get_at(user_data, 0);
                Num *id = num_new_cstr(s->cstr, true);
                map_set(user, "UID", id);
                s = list_get_at(user_data, 1);
                id = num_new_cstr(s->cstr, true);
                map_set(user, "GID", id);
                map_set(user, "User ID Info", list_remove_at(user_data, 2));
                map_set(user, "Home", list_remove_at(user_data, 2));
                map_set(user, "Shell", list_remove_at(user_data, 2));
                list_append(users, user);
            }
            delete(user_data);
            delete(line);
        }
        file_close(f);
    }
    Json *js = new(Json, users);
    json_pretty_print(js);
    delete_objs(js, f, users);
    return 0;
}
