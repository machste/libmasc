#include <masc/num.h>
#include <masc/math.h>
#include <masc/macro.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
    Double *d = new(Double, 42.0);
    put(d);
    double_set(d, 10.0 / 3);
    put(d);
    double_set_cstr(d, "0.345", true);
    put(d);
    Num *n1 = cstr_to_number("0xff", true, NULL);
    print("n1 = 0xff = %O (%s)\n", n1, name_of(n1));
    Num *n2 = new_copy(n1);
    print("n2 = n1 = %O (%s)\n", n2, name_of(n2));
    Num *n3 = add(n2, d);
    print("n3 = n2 + 0.345 = %O (%s)\n", n3, name_of(n3));
    Num *n4 = add(d, n3);
    print("n4 = 0.345 + n3 = %O (%s)\n", n4, name_of(n4));
    Int *i1 = int_new_cstr("0xdeadbeef", true);
    print("i1 = 0xdeadbeef = %O (%s)\n", i1, name_of(i1));
    int_set_cstr(i1, "12V", false);
    print("i1 = 12V = %O (%s)\n", i1, name_of(i1));
    iadd(n3, i1);
    print("n3 = n3 + i1 = %O (%s)\n", n3, name_of(n3));
    delete(d);
    delete(n1);
    delete(n2);
    delete(n3);
    delete(n4);
    delete(i1);
    char *cstr_nums[] = {"42", "0.12", "0xef", "0xf.dx", "gugus", "1A", "0.1B",
            "0b1001"};
    for (int i = 0; i < ARRAY_LEN(cstr_nums); i++) {
        Num *num = cstr_to_number(cstr_nums[i], false, NULL);
        Num *num_strict = cstr_to_number(cstr_nums[i], true, NULL);
        print("%s: %O (%s), strict: %O (%s)\n", cstr_nums[i], num, name_of(num),
                num_strict, name_of(num_strict));
        delete(num);
        delete(num_strict);
    }
    return 0;
}
