#include <masc/array.h>
#include <masc/str.h>
#include <masc/num.h>
#include <masc/iter.h>
#include <masc/math.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    Array *a1 = new(Array, sizeof(Str), 5);
    // Put empty array
    put(a1);
    // Set a string
    Str *s = new(Str, "Hallo Welt");
    array_copy_at(a1, 3, s);
    delete(s);
    put(a1);
    // Fill it with some strings
    for (int i = 0; i < array_len(a1); i++) {
        Str s = init(Str, "item %i", i);
        array_set_at(a1, i, &s);
    }
    put(a1);
    // Make a new copy of the first array
    Array *a2 = new_copy(a1);
    // Modify an object of the array
    s = array_get_at(a2, -1);
    str_fmt(s, "Array length: %i", array_len(a2));
    put(a2);
    bool _filter_cb(Str *s) {
        return str_len(s) < 10;
    }
    filter(a2, (filter_cb)_filter_cb);
    put(a2);
    // Mixed array
    Array *mixed = new(Array, max(sizeof(Str), sizeof(Num)), 6);
    // Fill it with strings and nums
    for (int i = 0; i < array_len(mixed); i++) {
        if (i % 2 == 0) {
            Str s = init(Str, "string %i", i);
            array_set_at(mixed, i, &s);
        } else {
            Num n = init(Num, (double)i);
            array_set_at(mixed, i, &n);
        }
    }
    put(mixed);
    // Destroy a object at a certain index
    array_destroy_at(mixed, 2);
    put(mixed);
    // Iterate over array
    Iter *itr = new(Iter, mixed);
    for (void *obj = next(itr); obj != NULL; obj = next(itr)) {
        Str *s = to_str(obj);
        print("%i: %O\n", iter_get_idx(itr), s);
        delete(s);
    }
    delete(itr);
    Str *joined = str_join(mixed, " <--> ");
    print("joined: %O\n", joined);
    // Clean-up
    delete(joined);
    delete(a1);
    delete(a2);
    delete(mixed);
    return 0;
}
