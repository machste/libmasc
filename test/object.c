#include <masc/object.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
    Object *o = new(Object);
    put(o);
    delete(o);
    return 0;
}
