#include <masc.h>


static bool accept_cb(TcpServer *self, TcpServerCli *cli)
{
    print("new: %O\n", cli);
    if (list_len(&self->clients) >= 2) {
        dprint(cli->fd, "Too many other clients, good bye!\n");
        print("%O: No new clients!\n", self);
        return false;
    }
    dprint(cli->fd, "Welcome to reversed, type something ...\n");
    return true;
}

static void pkg_cb(TcpServer *self, TcpServerCli *cli, void *data, size_t size)
{
    Str *str = str_new_ncopy(data, size);
    str_strip(str);
    print("data: %O: %O\n", cli, str);
    if (cstr_eq(str_cstr(str), "quit")) {
        dprint(cli->fd, "Good bye!\n", str);
        tcpserver_cli_close(cli);
    } else if (cstr_eq(str_cstr(str), "stop")) {
        dprint(cli->fd, "Shutdown reversed, good bye!\n", str);
        mloop_stop();
    } else {
        str_reverse(str);
        dprint(cli->fd, "%O\n", str);
    } 
    delete(str);
}

static void cli_hup_cb(TcpServer *self, TcpServerCli *cli)
{
    print("hang-up: %O\n", cli);
}

int main(int argc, char *argv[])
{
    int ret = 0;
    TcpServer server = init(TcpServer, "0.0.0.0", 8080);
    server.accept_cb = accept_cb;
    server.cli_packet_cb = pkg_cb;
    server.cli_hup_cb = cli_hup_cb;
    mloop_init();
    TcpServerError err = tcpserver_start(&server);
    if (err == TCPSERVER_SUCCESS) {
        print("%O: waiting for connections ...\n", &server);
        mloop_run();
        print("%O: shutdown\n", &server);
    } else {
        fprint(stderr, "Error: unable to start reversed!\n");
        ret = -1;
    }
    destroy(&server);
    mloop_destroy();
    return ret;
}
