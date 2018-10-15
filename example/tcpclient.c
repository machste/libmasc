#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <masc.h>


static TcpClient client;


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

static void stdin_line_cb(Str *line)
{
    dprint(client.fd, "%O\n", line);
}

static void stdin_cb(MlFd *self, int fd, ml_fd_flag_t events, void *arg)
{
    static char *buf = NULL;
    static size_t buf_size = 0;
    static size_t pos = 0;
    // Read data
    if (events & ML_FD_READ) {
        while(true) {
            if (buf_size <= pos) {
                buf_size += 256;
                buf = realloc(buf, buf_size);
            }
            ssize_t len = read(fd, buf + pos, buf_size - pos);
            if (len > 0) {
                pos += len;
            } else {
                break;
            }
        }
    }
    // Search for complete lines
    size_t i = 0, start = 0;
    for (i = 0; i < pos; i++) {
        if (buf[i] == '\n') {
            Str *str = str_new_ncopy(buf + start, i - start);
            stdin_line_cb(str);
            delete(str);
            // Set new start posistion and skip new line
            start = i + 1;
        }
    }
    // Copy remaining data to the start of the buffer
    if (start < i) {
        pos = i - start;
        memcpy(buf, buf + start, pos);
    } else {
        free(buf);
        buf = NULL;
        pos = 0;
        buf_size = 0;
    }
    if (events & ML_FD_EOF) {
        log_error("stdin eof!");
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
    argparse_set_default(ap, "port", "8080");
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
    client = init(TcpClient, str_cstr(ip), port);
    client.cli_packet_cb = pkg_cb;
    client.serv_hup_cb = serv_hup_cb;
    // Init mloop
    mloop_init();
    mloop_fd_new(STDIN_FILENO, ML_FD_READ, stdin_cb, NULL);
    TcpClientError err = tcpclient_start(&client);
    if (err == TCPCLIENT_SUCCESS) {
        log_debug("%O: connected.", &client);
        mloop_run();
    } else {
        log_error("unable to connect to %O:%i!", ip, port);
        ret = -1;
    }
    delete(ip);
    destroy(&client);
    mloop_destroy();
    log_destroy();
    return ret;
}
