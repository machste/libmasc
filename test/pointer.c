#include <masc/pointer.h>
#include <masc/map.h>
#include <masc/str.h>
#include <masc/print.h>


typedef Str *(*str_func)(Str *self);


int main(int argc, char *argv[])
{
    Map *str_funcs = new(Map);
    map_set(str_funcs, "swapcase", new(Pointer, str_swapcase));
    map_set(str_funcs, "lower", new(Pointer, str_lower));
    map_set(str_funcs, "upper", new(Pointer, str_upper));
    map_set(str_funcs, "snakecase", new(Pointer, str_snakecase));
    print("string functions: %O\n", str_funcs);
    Str *s = str_new_cstr("Hello-World");
    Iter i = init(Iter, str_funcs);
    for (Pointer *p = next(&i); p != NULL; p = next(&i)) {
        print("s: %O\n", s);
        str_func func = p->ptr;
        Str *result = func(s);
        print("call '%s' result: %O\n", iter_get_key(&i), result);
    }
    destroy(&i);
    delete(s);
    delete(str_funcs);
    return 0;
}