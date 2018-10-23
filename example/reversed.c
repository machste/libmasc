#include <masc.h>


static bool accept_cb(TcpServer *self, TcpServerCli *cli)
{
    log_info("new: %O", cli);
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
    log_debug("data: %O: %O", cli, str);
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
    log_info("hang-up: %O\n", cli);
}

static void *log_level_check(Str *log_level_str, Str **err_msg)
{
    Int *log_level = argparse_int(log_level_str, err_msg);
    if (log_level != NULL) {
        if (!int_in_range(log_level, 0, 7)) {
            *err_msg = str_new("invalid log level: %O!", log_level_str);
            delete(log_level);
            log_level = NULL;
        }
    }
    return log_level;
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
    const char *prog = path_basename(argv[0]);
    // Setup argument parser
    Argparse *ap = new(Argparse, prog, "Reverse Daemon");
    argparse_add_opt(ap, 'l', "log-level", "LEVEL", "1", log_level_check,
            "log level (0 - 7)");
    argparse_set_default(ap, "log-level", "6");
    argparse_add_opt(ap, 'b', NULL, NULL, NULL, NULL, "run as daemon");
    argparse_add_opt(ap, 0, "bind", "IP", "1", argparse_ip, "IP address");
    argparse_set_default(ap, "bind", "0.0.0.0");
    argparse_add_arg(ap, "port", "PORT", "?", port_check, "port (0 - 65535)");
    argparse_set_default(ap, "port", "4321");
    // Parse command line arguments
    Map *args = argparse_parse(ap, argc, argv);
    delete(ap);
    int log_level = int_get(map_get(args, "log-level"));
    bool daemonize = bool_get(map_get(args, "b"));
    Str *ip = map_remove_key(args, "bind");
    int port = int_get(map_get(args, "port"));
    delete(args);
    // Setup logging
    log_init(log_level);
    if (daemonize) {
        log_add_syslog(prog, 0, LOG_DAEMON);
    } else {
        log_add_stdout();
    }
    // Setup TCP server
    TcpServer server = init(TcpServer, str_cstr(ip), port);
    delete(ip);
    server.accept_cb = accept_cb;
    server.cli_pkg_cb = pkg_cb;
    server.cli_hup_cb = cli_hup_cb;
    mloop_init();
    TcpServerError err = tcpserver_start(&server);
    if (err == TCPSERVER_SUCCESS) {
        if (daemonize) {
            daemon(0, 0);
        }
        log_info("%O: waiting for connections ...", &server);
        mloop_run();
        log_info("%O: shutdown", &server);
    } else {
        log_error("unable to start reversed!");
        ret = -1;
    }
    destroy(&server);
    mloop_destroy();
    log_destroy();
    return ret;
}
