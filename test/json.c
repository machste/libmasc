#include <masc/json.h>
#include <masc/int.h>
#include <masc/list.h>
#include <masc/map.h>
#include <masc/macro.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
    Json *js = json_new_cstr("[1, 3.12, \"string\", {\"1\": 1, \"2\": false}]");
    put_repr(js);
    put(js);
    json_parse(js, "{\"node\":{\"a\":[[null, -0.1, {\"foo\":\"bar\", \"a.b\": 666}], 42], \"utf-8\":\"Mächler\nStefan '\\u00a3, \\u0418, \\u20ac'\", \"bad\":\"\xf0\x9d\"}}");
    json_pretty_print(js);
    // Get node
    char *get_keys[] = {"", "node.a[0][0]", "node", "[\"node\"][\"a\"][1]",
        "a[b.c]", "node.a[0][1]", "node.a[0][2].foo", "node.a[0][2][\"a.b\"]",
        "gugus", "[\"node\"]", "node.utf-8"
    };
    for (int i = 0; i < ARRAY_LEN(get_keys); i++) {
        void *node = json_get_node(js, get_keys[i]);
        print("get '%s': %O (%s)\n", get_keys[i], node, name_of(node));
    }
    void *root_backup = new_copy(json_get_node(js, ""));
    // Set node
    char *set_keys[] = {"node.a[0][0]", "node[\"a\"][0][2].set", "gugus",
        "node.a.bar"
    };
    for (int i = 0; i < ARRAY_LEN(set_keys); i++) {
        Int *new_value = new(Int, i);
        print("set '%s': ", set_keys[i]);
        if (json_set_node(js, set_keys[i], new_value)) {
            put(js);
        } else {
            puts("failed!");
            delete(new_value);
        }
    }
    json_set_node(js, "", root_backup);
    print("restore root: %O\n", js);
    // Remove and delete node
    void *node = json_remove_node(js, "node.a[0][1]");
    print("removed node.a[0][1]: %O (%s)\n", node, name_of(node));
    delete(node);
    put(js);
    json_delete_node(js, "");
    print("deleted root: %O\n", js);
    json_set_root(js, new(Map));
    json_set_node(js, "list", new(List));
    json_set_node(js, "list[5]", new(Int, 5));
    json_set_node(js, "list[3]", new(Int, 3));
    put(js);
    delete(js);
}
