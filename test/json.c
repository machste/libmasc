#include <masc/json.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
    Json *js = json_new_cstr("[1, 3.12, \"string\", {\"1\": 1, \"2\": false}]");
    put_repr(js);
    put(js);
    delete(js);
}
