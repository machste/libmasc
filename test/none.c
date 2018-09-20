#include <masc/none.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    void *none = new(NULL);
    print("%s: %O (cstr)\n", name_of(none), none);
    delete(none);
    None none2 = init(None);
    char none2_repr[16];
    repr(&none2, none2_repr, sizeof(none2_repr));
    print("%s: %s (repr)\n", name_of(&none2), none2_repr);
    destroy(&none2);
    // Tests with NULL pointer
    print("name_of(NULL): %s, %O\n", name_of(NULL), NULL);
    print("print(\"%%O\\n\", NULL): %O\n", NULL);
    print("put_repr(NULL): ");
    put_repr(NULL);
    return 0;
}
