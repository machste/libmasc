#include <masc/num.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
    Num *n = new(Num, 42.0);
    put(n);
    num_set(n, 10.0 / 3);
    put(n);
    Num *n1 = num_new(1000);
    put(n1);
    Num *n2 = new_copy(n1);
    Num *n3 = num_add(n2, n);
    put(n2);
    put(n3);
    num_iadd(n3, n1);
    put(n3);
    delete(n);
    delete(n1);
    delete(n2);
    delete(n3);
    return 0;
}
