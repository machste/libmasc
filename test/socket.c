#include <masc/socket.h>
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
    Socket *sock = new(Socket, AF_INET, SOCK_STREAM, 0);
    test_io("sock", sock);
    delete(sock);
    return 0;
}
