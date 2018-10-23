#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <masc.h>


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

static void *host_check(Str *hostname, Str **err_msg)
{
    Str *ip = NULL;
    struct hostent *host = gethostbyname(str_cstr(hostname));
    if (host != NULL) {
        ip = str_new_cstr(inet_ntoa(*((struct in_addr *)host->h_addr)));
    } else {
        *err_msg = str_new("unknown host: %O!", hostname);
    }
    return ip;
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

void stdin_line_cb(MlFdPkg *self, void *data, size_t size, void *arg)
{
    TcpClient *cli = arg;
    write(cli->fd, data, size);
}

static void connect_cb(TcpClient *self, int so_errno)
{
    if (so_errno != 0) {
        log_error("unable to connect to %s:%i, %s!", tcpclient_ip(self),
                tcpclient_port(self), strerror(so_errno));
        mloop_stop();
    } else {
        log_debug("connected to %s:%i.", tcpclient_ip(self),
                tcpclient_port(self));
    }
}

static void pkg_cb(TcpClient *self, void *data, size_t size)
{
    Str *str = str_new_ncopy(data, size);
    str_strip(str);
    put(str);
    delete(str);
}

static void serv_hup_cb(TcpClient *self)
{
    log_error("Connection closed by remote host!", self);
    mloop_stop();
}

int main(int argc, char *argv[])
{
    int ret = 0;
    const char *prog = path_basename(argv[0]);
    // Setup argument parser
    Argparse *ap = new(Argparse, prog, "TCP Client");
    argparse_add_opt(ap, 'l', "log-level", "LEVEL", "1", log_level_check,
            "log level (0 - 7)");
    argparse_set_default(ap, "log-level", "6");
    argparse_add_arg(ap, "ip", "HOST", NULL, host_check, "hostname");
    argparse_add_arg(ap, "port", "PORT", "?", port_check, "port (0 - 65535)");
    argparse_set_default(ap, "port", "4321");
    // Parse command line arguments
    Map *args = argparse_parse(ap, argc, argv);
    delete(ap);
    int log_level = int_get(map_get(args, "log-level"));
    Str *ip = map_remove_key(args, "ip");
    int port = int_get(map_get(args, "port"));
    delete(args);
    // Setup logging
    log_init(log_level);
    log_add_stdout();
    // Setup TCP client
    TcpClient client = init(TcpClient, str_cstr(ip), port);
    client.connect_cb = connect_cb;
    client.pkg_cb = pkg_cb;
    client.serv_hup_cb = serv_hup_cb;
    delete(ip);
    // Init mloop
    mloop_init();
    mloop_fd_pkg_new(STDIN_FILENO, '\n', stdin_line_cb, NULL, &client);
    // Start TCP client
    if (tcpclient_start(&client) == TCPCLIENT_SUCCESS) {
        mloop_run();
    } else {
        ret = -1;
    }
    destroy(&client);
    mloop_destroy();
    log_destroy();
    return ret;
}
