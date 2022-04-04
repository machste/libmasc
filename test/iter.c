#include <masc/list.h>
#include <masc/object.h>
#include <masc/str.h>
#include <masc/iter.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    List *l = new(List);
    Object *o = new(Object);
    list_append(l, o);
    Str *str1 = new(Str, "str1");
    list_append(l, str1);
    list_append(l, new(Str, "str2"));
    put(l);
    print("len(o): %zu\n", len(o));
    print("len(l): %zu\n", len(l));
    delete(l);
    return 0;
}



