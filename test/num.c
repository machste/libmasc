#include <masc/num.h>
#include <masc/int.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
    Num *n = new(Num, 42.0);
    put(n);
    num_set(n, 10.0 / 3);
    put(n);
    num_set_cstr(n, "0.345");
    print("%O (%s)\n", n, name_of(n));
    Num *n1 = num_new_cstr("0xff");
    put(n1);
    Num *n2 = new_copy(n1);
    Num *n3 = num_add(n2, n);
    put(n2);
    put(n3);
    num_iadd(n3, n1);
    put(n3);
    Int *i1 = int_new_cstr("0xdeadbeef");
    print("%O (%s)\n", i1, name_of(i1));
    int_set_cstr(i1, "12V");
    put(i1);
    delete(n);
    delete(n1);
    delete(n2);
    delete(n3);
    delete(i1);
    return 0;
}
