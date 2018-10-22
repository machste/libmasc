#include <masc/io.h>
#include <masc/print.h>


static void test_io(const char *name, Io *io)
{
    print("%s: %O\n", name, io);
    print(" * is_open: %s\n", is_open(io) ? "true" : "false");
    print(" * is_blocking: %s\n", is_blocking(io) ? "true" : "false");
    print(" * is_readable: %s\n", is_readable(io) ? "true" : "false");
    print(" * is_writable: %s\n", is_writable(io) ? "true" : "false");
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    Io *in = new(Io, STDIN_FILENO);
    Io *out = new(Io, STDOUT_FILENO);
    Io invalid = init(Io, -1234);
    test_io("in", in);
    test_io("out", out);
    test_io("invalid", &invalid);
    destroy(&invalid);
    const char test_cstr[] = "write: Hello World!\n";
    write(out, test_cstr, sizeof(test_cstr));
    writefmt(out, "write_fmt(out, \"out: %%O\\n\"): out: %O\n", out);
    Io *copy = new_copy(out);
    writefmt(copy, "write_fmt(copy, \"copy: %%O\\n\"): copy: %O\n", copy);
    delete(copy);
    delete(out);
    delete(in);
    return 0;
}
