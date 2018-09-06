#include <masc/map.h>
#include <masc/num.h>
#include <masc/bool.h>
#include <masc/str.h>
#include <masc/list.h>
#include <masc/iter.h>
#include <masc/macro.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    Map *m = new(Map);
    // Fill the map with some values
    map_set(m, "number", new(Num, 41.0));
    map_set(m, "string", new(Str, "Hallo Welt"));
    map_set(m, "number", new(Num, 42.0));
    map_set(m, "bool", new(Bool, true));
    map_set(m, "float", new(Num, 3.333));
    map_set(m, "values", map_get_values(m));
    print("map: %O, len: %zu\n", m, map_len(m));
    // Iterate over map
    Iter *itr = new(Iter, m);
    for (void *obj = next(itr); obj != NULL; obj = next(itr)) {
        if (isinstance(obj, List)) {
            print("%i: key: %s, value: (%s)\n", iter_get_idx(itr),
                    iter_get_key(itr), name_of(obj));
            Iter *li = new(Iter, obj);
            for (void *lo = next(li); lo != NULL; lo = next(li)) {
                print(" * %i: %O (%s)\n", iter_get_idx(li), lo, name_of(lo));
            }
            delete(li);
        } else {
            print("%i: key: %s, value: %O (%s)\n", iter_get_idx(itr),
                    iter_get_key(itr), obj, name_of(obj));
        }
    }
    delete(itr);
    // Delete key "values"
    map_delete_key(m, "values");
    print("map: %O\n", m);
    // Filter map
    filter(m, LAMBDA(bool, (void *o){return !isinstance(o, Num);}));
    print("map: %O\n", m);
    delete(m);
    return 0;
}
