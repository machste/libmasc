#include <masc/map.h>
#include <masc/num.h>
#include <masc/str.h>
#include <masc/list.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    Map *m = new(Map);
    // Fill the map with some values
    map_set(m, "number", new(Num, 41.0));
    map_set(m, "string", new(Str, "Hallo Welt"));
    map_set(m, "number", new(Num, 42.0));
    put(m);
    // Iterate over map
    Iter i = map_iter(m);
    for (void *obj = next(&i); obj != NULL; obj = next(&i)) {
        print("%zu: key: %s, value: %O\n", i.index, i.key, obj);
    }
    print("map: %O\n", m);
    delete(m);
    return 0;
}
