#include <masc/none.h>
#include <masc/print.h>


int main(int argc, char *argv[])
{
    void *none = new(NULL);
    print("%s: %O\n", name_of(none), none);
    delete(none);
    None none2 = init(None);
    print("%s: %O\n", name_of(&none2), &none2);
    destroy(&none2);
    return 0;
}
