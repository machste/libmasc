#include <masc/list.h>
#include <masc/num.h>
#include <masc/bool.h>
#include <masc/str.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    List *l1 = new(List);
    print("get_at(0): %O\n", list_get_at(l1, 0));
    // Fill the list with some numbers
    for (double d = 0; d < 10; d++) {
        Num *n = new(Num, d);
        list_append(l1, n);
    }
    print("l1: %O (len: %zu)\n", l1, list_len(l1));
    print("get_at(-3): %O, get_at(4): %O\n", list_get_at(l1, -3),
            list_get_at(l1, 4));
    // Iterate over list
    Iter i = list_iter(l1);
    for (Num *n = next(&i); n != NULL; n = next(&i)) {
        print("%s: %O\n", name_of(n), n);
    }
    // Copy list
    List l2;
    init_copy(&l2, l1);
    list_append(&l2, new(Str, "foobar"));
    list_append(&l2, NULL);
    list_append(&l2, new(Bool, false));
    list_set_at(&l2, 4, new(Str, "4"));
    put(&l2);
    list_insert_after(&l2, 2, new(Num, 2.5));
    list_insert_at(&l2, 0, new(Num, 0.5));
    list_insert_at(&l2, -1, new(Bool, true));
    // For each item
    void item_cb(void *obj) {
        print("%s: %O\n", name_of(obj), obj);
    }
    list_for_each(&l2, item_cb);
    void *obj = list_remove_at(l1, 5);
    print("list_remove_at(l1, 5): %s: %O\n", name_of(obj), obj);
    delete(obj);
    list_delete_at(l1, 0);
    list_delete_at(l1, -1);
    put(l1);
    delete(l1);
    destroy(&l2);
    return 0;
}
