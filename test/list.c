#include <masc/list.h>
#include <masc/num.h>
#include <masc/bool.h>
#include <masc/str.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    List *l1 = new(List);
    // Fill the list with some numbers
    for (double d = 0; d < 10; d++) {
        Num *n = new(Num, d);
        list_append(l1, n);
    }
    put(l1);
    // Iterate over list
    Iter i = list_iter(l1);
    for (Num *n = next(&i); n != NULL; n = next(&i)) {
        put(n);
    }
    // Copy list
    List l2;
    init_copy(&l2, l1);
    list_append(&l2, new(Str, "foobar"));
    list_append(&l2, NULL);
    list_append(&l2, new(Bool, false));
    put(&l2);
    // Iterate over list
    i = list_iter(&l2);
    for (void *obj = next(&i); obj != NULL; obj = next(&i)) {
        put(obj);
    }
    delete(l1);
    destroy(&l2);
    return 0;
}
