#include <masc/list.h>
#include <masc/double.h>
#include <masc/bool.h>
#include <masc/char.h>
#include <masc/str.h>
#include <masc/iter.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    List *l1 = new(List);
    print("get_at(0): %O\n", list_get_at(l1, 0));
    // Fill the list with some numbers
    for (double d = 0; d < 10; d++) {
        Double *n = new(Double, d);
        list_append(l1, n);
    }
    print("l1: %O (len: %zu)\n", l1, len(l1));
    print("get_at(-3): %O, get_at(4): %O\n", list_get_at(l1, -3),
            list_get_at(l1, 4));
    // Iterate over list
    Iter *itr = new(Iter, l1);
    for (Double *n = next(itr); n != NULL; n = next(itr)) {
        print("%s: %O (idx: %i)\n", name_of(n), n, iter_get_idx(itr));
    }
    delete(itr);
    // Copy list
    List l2;
    init_copy(&l2, l1);
    list_append(&l2, new(Str, "foobar"));
    list_append(&l2, NULL);
    list_append(&l2, new(Str, "abc64"));
    list_append(&l2, new(Bool, false));
    list_set_at(&l2, 4, new(Char, '4'));
    print("l2: %O\n", &l2);
    list_insert_after(&l2, 2, new(Double, 2.5));
    list_insert_at(&l2, 0, new(Double, 0.5));
    list_insert_at(&l2, -1, new(Bool, true));
    // For each item
    void *obj_cb(void *obj, double *min) {
        print("%s: %O\n", name_of(obj), obj);
        if (isinstance(obj, Double) && double_get(obj) > *min) {
            return obj;
        } else {
            return NULL;
        }
    }
    void *b = list_for_each(&l2, (list_obj_cb)obj_cb, &(double){ 7.5 });
    print("Found number: %O\n", b);
    // Sort list
    list_sort(&l2, cmp);
    print("sorted l2: %O\n", &l2);
    List *l3 = new(List);
    list_append(l3, new(Str, "Mouse"));
    list_append(l3, new(Str, "Cat"));
    list_append(l3, new(Str, "Bison"));
    list_append(l3, new(Str, "Rat"));
    print("l3: %O\n", l3);
    list_sort(l3, NULL);
    print("sorted l3: %O\n", l3);
    // Sort in
    Str *hound = new(Str, "Hound");
    print("is 'Hound' in list: %s\n", list_is_in(l3, hound) ? "true" : "false");
    list_sort_in(l3, hound, NULL);
    print("sort 'Hound' into l3: %O\n", l3);
    print("is 'Hound' in list: %s\n", list_is_in(l3, hound) ? "true" : "false");
    // Delete object from list
    list_delete_obj(l3, hound);
    print("delete 'Hound' from l3: %O\n", l3);
    // Remove at index
    void *obj = list_remove_at(l1, 5);
    print("list_remove_at(l1, 5): %s: %O\n", name_of(obj), obj);
    delete(obj);
    list_delete_at(l1, 0);
    list_delete_at(l1, -1);
    print("l1: %O\n", l1);
    // Copy l3 to l1
    list_copy(l1, l3);
    print("l1: %O\n", l1);
    delete(l3);
    delete(l1);
    destroy(&l2);
    return 0;
}
