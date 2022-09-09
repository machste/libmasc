#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <masc/tcpserver.h>
#include <masc/mloop.h>


static const void *TcpServerCliCls;


void tcpserver_init(TcpServer *self, const char *ip, int port)
{
    object_init(self, TcpServerCls);
    self->ip = strdup(ip);
    self->port = port;
    self->sock = NULL;
    self->listen_backlog = 4;
    self->sentinel = '\n';
    self->accept_cb = NULL;
    self->cli_data_cb = NULL;
    self->cli_pkg_cb = NULL;
    self->cli_hup_cb = NULL;
    list_init(&self->clients);
    self->err = TCPSERVER_SUCCESS;
}

static void _vinit(TcpServer *self, va_list va)
{
    char *ip = va_arg(va, char *);
    int port = va_arg(va, int);
    tcpserver_init(self, ip, port);
}

void tcpserver_destroy(TcpServer *self)
{
    free(self->ip);
    list_destroy(&self->clients);
    if (self->sock != NULL) {
        MlIo *mlio = mloop_io_by_io(self->sock);
        if (mlio != NULL) {
            mloop_io_delete(mlio);
        }
        delete(self->sock);
    }
}

static void _cli_data_cb(MlIoReader *self, void *data, size_t size, void *arg)
{
    TcpServerCli *cli = arg;
    cli->server->cli_data_cb(cli->server, cli, data, size);
}

static void _cli_pkg_cb(MlIoPkg *self, void *data, size_t size, void *arg)
{
    TcpServerCli *cli = arg;
    cli->server->cli_pkg_cb(cli->server, cli, data, size);
}

static void _eof_cb(MlIoReader *self, void *arg)
{
    TcpServerCli *cli = arg;
    if(cli->server->cli_hup_cb != NULL) {
        cli->server->cli_hup_cb(cli->server, cli);
    }
    tcpserver_cli_close(cli);
}

static void _cli_reg_data_callbacks(TcpServer *self, TcpServerCli *cli)
{
    if (self->cli_data_cb != NULL) {
        mloop_io_reader_new(cli->sock, _cli_data_cb, _eof_cb, cli);
    } else if (self->cli_pkg_cb != NULL) {
        mloop_io_pkg_new(cli->sock, self->sentinel, _cli_pkg_cb, _eof_cb, cli);
    } else {
        mloop_io_reader_new(cli->sock, NULL, _eof_cb, cli);
    }
}

static void _server_cb(MlIo *self, int fd, ml_io_flag_t events, void *arg)
{
    TcpServer *server = arg;
    Socket *s;
    struct sockaddr in_addr;
    socklen_t in_len = sizeof(in_addr);
    while ((s = socket_accept((Socket *)self->io, &in_addr, &in_len)) != NULL) {
        TcpServerCli *cli = new(TcpServerCli, server, s, &in_addr);
        bool accept_cli = true;
        if(server->accept_cb != NULL) {
            accept_cli = server->accept_cb(server, cli);
        }
        if (accept_cli) {
            _cli_reg_data_callbacks(server, cli);
            list_append(&server->clients, cli);
        } else {
            delete(cli);
        }
        in_len = sizeof(in_addr);
    }
}

TcpServerError tcpserver_start(TcpServer *self)
{
    if (self->err != TCPSERVER_SUCCESS) {
        return self->err;
    }
    if (self->sock != NULL) {
        self->err = TCPSERVER_SOCKET_IN_USE;
        return self->err;
    }
    Socket *sock = new(Socket, AF_INET, SOCK_STREAM, 0);
    if (!is_open(sock)) {
        self->err = TCPSERVER_SOCKET_ERR;
        return self->err;
    }
    int o = 1;
    if (socket_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
            &o, sizeof(o)) < 0) {
        delete(sock);
        self->err = TCPSERVER_SETSOCKET_ERR;
        return self->err;
    }
    if (!socket_bind(sock, self->ip, self->port)) {
        delete(sock);
        self->err = TCPSERVER_BIND_ERR;
        return self->err;
    }
    if (!socket_listen(sock, self->listen_backlog)) {
        delete(sock);
        self->err = TCPSERVER_LISTEN_ERR;
        return self->err;
    }
    // Use socket in the mloop
    self->sock = sock;
    mloop_io_new(self->sock, ML_IO_READ, _server_cb, self);
    return self->err;
}

void tcpserver_cli_close(TcpServerCli *cli)
{
    list_delete_obj(&cli->server->clients, cli);
}

size_t tcpserver_to_cstr(TcpServer *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s %s:%u at %p>", name_of(self), self->ip,
            self->port, self);
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static class _TcpServerCls = {
    .name = "TcpServer",
    .size = sizeof(TcpServer),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)tcpserver_destroy,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)tcpserver_to_cstr,
    .to_cstr = (to_cstr_cb)tcpserver_to_cstr,
};

const void *TcpServerCls = &_TcpServerCls;


static void _cli_vinit(TcpServerCli *self, va_list va)
{
    object_init(self, TcpServerCliCls);
    self->server = va_arg(va, TcpServer *);
    self->sock = va_arg(va, Socket *);
    struct sockaddr_in *addr = va_arg(va, struct sockaddr_in *);
    memcpy(&self->addr, addr, sizeof(self->addr));
}

static void _cli_destroy(TcpServerCli *self, va_list va)
{
    MlIo *mlio = mloop_io_by_io(self->sock);
    if (mlio != NULL) {
        mloop_io_delete(mlio);
    }
    delete(self->sock);
}

static size_t _cli_to_cstr(TcpServerCli *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s %s:%u at %p>", name_of(self),
            inet_ntoa(self->addr.sin_addr), ntohs(self->addr.sin_port), self);
}


static class _TcpServerCliCls = {
    .name = "TcpServerCli",
    .size = sizeof(TcpServerCli),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_cli_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)_cli_destroy,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)_cli_to_cstr,
    .to_cstr = (to_cstr_cb)_cli_to_cstr,
};

static const void *TcpServerCliCls = &_TcpServerCliCls;
