#include <masc/json.h>
#include <masc/macro.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
    Json *js = json_new_cstr("[1, 3.12, \"string\", {\"1\": 1, \"2\": false}]");
    put_repr(js);
    put(js);
    json_parse(js, "{\"node\":{\"a\":[[null, -0.1, {\"foo\":\"bar\"}], 42]}}");
    json_pretty_print(js);
    char *keys[] = {"node.a[0][0]", "node", "node.a[1]", "node.a[0][1]",
        "node.a[0][2].foo", "gugus"
    };
    for (int i = 0; i < ARRAY_LEN(keys); i++) {
        void *node = json_get_node(js, keys[i]);
        print("%s: %O (%s)\n", keys[i], node, name_of(node));
    }
    delete(js);
}
