#include <masc/map.h>
#include <masc/num.h>
#include <masc/str.h>
#include <masc/list.h>
#include <masc/iter.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    Map *m = new(Map);
    // Fill the map with some values
    map_set(m, "number", new(Num, 41.0));
    map_set(m, "string", new(Str, "Hallo Welt"));
    map_set(m, "number", new(Num, 42.0));
    print("map: %O, len: %zu\n", m, map_len(m));
    // Iterate over map
    Iter *itr = new(Iter, m);
    for (void *obj = next(itr); obj != NULL; obj = next(itr)) {
        print("%i: key: %s, value: %O\n", iter_get_idx(itr),
                iter_get_key(itr), obj);
    }
    delete(itr);
    delete(m);
    return 0;
}
