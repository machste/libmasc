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

static void *port_check(Str *port_str, Str **err_msg)
{
    Int *port = argparse_int(port_str, err_msg);
    if (port != NULL) {
        if (!int_in_range(port, 0, 65535)) {
            *err_msg = str_new("invalid port number: %O!", port_str);
            delete(port);
            port = NULL;
        }
    }
    return port;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    // Setup argument parser
    Argparse *ap = new(Argparse, path_basename(argv[0]), "Reverse Daemon");
    argparse_add_opt(ap, 0, "bind", "IP", "1", argparse_ip, "IP address");
    argparse_set_default(ap, "bind", "0.0.0.0");
    argparse_add_arg(ap, "port", "PORT", "?", port_check, "port (0 - 65535)");
    argparse_set_default(ap, "port", "8080");
    // Parse command line arguments
    Map *args = argparse_parse(ap, argc, argv);
    put(args);
    delete(ap);
    Str *ip = map_get(args, "bind");
    int port = int_get(map_get(args, "port"));
    TcpServer server = init(TcpServer, str_cstr(ip), port);
    delete(args);
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
