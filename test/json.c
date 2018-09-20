#include <masc/json.h>
#include <masc/macro.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
    Json *js = json_new_cstr("[1, 3.12, \"string\", {\"1\": 1, \"2\": false}]");
    put_repr(js);
    put(js);
    json_parse(js, "{\"node1\":{\"node2\":[[null, {\"foo\":\"bar\"}], 42]}}");
    json_pretty_print(js);
    char *keys[] = {"node1", "node1.node2[1]", "node1.node2[0][1].foo",
        "node1.node2[0][0]", "gugus"
    };
    for (int i = 0; i < ARRAY_LEN(keys); i++) {
        void *node = json_get_node(js, keys[i]);
        print("%s: %O (%s)\n", keys[i], node, name_of(node));
    }
    delete(js);
}
