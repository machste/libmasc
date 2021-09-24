#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <masc/tcpclient.h>


void tcpclient_init(TcpClient *self, const char *ip, int port)
{
    object_init(self, TcpClientCls);
    self->ip = strdup(ip);
    self->port = port;
    self->sock = NULL;
    self->sentinel = '\n';
    self->timeout = 3000;
    self->conn_evt = NULL;
    self->time_evt = NULL;
    self->connect_cb = NULL;
    self->data_cb = NULL;
    self->pkg_cb = NULL;
    self->serv_hup_cb = NULL;
    self->err = TCPCLIENT_SUCCESS;
}

static void _vinit(TcpClient *self, va_list va)
{
    char *ip = va_arg(va, char *);
    int port = va_arg(va, int);
    tcpclient_init(self, ip, port);
}

void tcpclient_destroy(TcpClient *self)
{
    free(self->ip);
    tcpclient_stop(self);
}

static void _connect_cb(TcpClient *self, int so_errno)
{
    if (self->connect_cb != NULL) {
        if (so_errno == 0) {
            self->connect_cb(self, NULL);
        } else {
            self->connect_cb(self, strerror(so_errno));
        }
    }
}

static void _data_cb(MlIoReader *self, void *data, size_t size, void *arg)
{
    TcpClient *cli = arg;
    cli->data_cb(cli, data, size);
}

static void _pkg_cb(MlIoPkg *self, void *data, size_t size, void *arg)
{
    TcpClient *cli = arg;
    cli->pkg_cb(cli, data, size);
}

static void _eof_cb(MlIoReader *self, void *arg)
{
    TcpClient *cli = arg;
    cli->err = TCPCLIENT_SERV_HUP_ERR;
    if(cli->serv_hup_cb != NULL) {
        cli->serv_hup_cb(cli);
    }
    tcpclient_stop(cli);
}

static void _reg_data_callbacks(TcpClient *self)
{
    if (self->data_cb != NULL) {
        mloop_io_reader_new(self->sock, _data_cb, _eof_cb, self);
    } else if (self->pkg_cb != NULL) {
        mloop_io_pkg_new(self->sock, self->sentinel, _pkg_cb, _eof_cb, self);
    } else {
        mloop_io_reader_new(self->sock, NULL, _eof_cb, self);
    }
}

static void _conn_evt(MlIo *self, int fd, ml_io_flag_t events, void *arg)
{
    TcpClient *cli = arg;
    mloop_io_delete(cli->conn_evt);
    cli->conn_evt = NULL;
    int so_err;
    socklen_t so_err_len = sizeof(so_err);
    socket_getsockopt(cli->sock, SOL_SOCKET, SO_ERROR, &so_err, &so_err_len);
    _connect_cb(cli, so_err);
    if (so_err == 0) {
        _reg_data_callbacks(cli);
        if (cli->time_evt != NULL) {
            mloop_timer_delete(cli->time_evt);
            cli->time_evt = NULL;
        }
    } else {
        cli->err = TCPCLIENT_CONNECT_ERR;
        delete(cli->sock);
        cli->sock = NULL;
    }
}

static void _timeout_cb(MlTimer *self, void *arg)
{
    TcpClient *cli = arg;
    mloop_io_delete(cli->conn_evt);
    cli->conn_evt = NULL;
    mloop_timer_delete(cli->time_evt);
    cli->time_evt = NULL;
    cli->err = TCPCLIENT_CONNECT_ERR;
    _connect_cb(cli, ETIMEDOUT);
    delete(cli->sock);
    cli->sock = NULL;
}

TcpClientError tcpclient_start(TcpClient *self)
{
    if (self->err != TCPCLIENT_SUCCESS) {
        return self->err;
    }
    self->sock = new(Socket, AF_INET, SOCK_STREAM, 0);
    if (!is_open(self->sock)) {
        self->err = TCPCLIENT_SOCKET_ERR;
        return self->err;
    }
    set_blocking(self->sock, false);
    // Try to connect
    bool in_progress = false;
    if (socket_connect(self->sock, self->ip, self->port)) {
        _reg_data_callbacks(self);
    } else if (errno == EINPROGRESS) {
        // Connecting process needs more time
        in_progress = true;
        self->conn_evt = mloop_io_new(self->sock, ML_IO_WRITE, _conn_evt, self);
        if (self->timeout > 0) {
            self->time_evt = mloop_timer_new(self->timeout, _timeout_cb, self);
        }
    } else {
        self->err = TCPCLIENT_CONNECT_ERR;
        delete(self->sock);
        self->sock = NULL;
        return self->err;
    }
    if (!in_progress) {
        _connect_cb(self, errno);
    }
    return self->err;
}

void tcpclient_stop(TcpClient *self)
{
    if (self->conn_evt != NULL) {
        mloop_io_delete(self->conn_evt);
    }
    if (self->time_evt != NULL) {
        mloop_timer_delete(self->time_evt);
    }
    if (self->sock != NULL) {
        MlIo *mlio = mloop_io_by_io(self->sock);
        if (mlio != NULL) {
            mloop_io_delete(mlio);
        }
        delete(self->sock);
        self->sock = NULL;
    }
}

size_t tcpclient_to_cstr(TcpClient *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s %s:%i at %p>", name_of(self), self->ip,
            self->port, self);
}


class _TcpClientCls = {
    .name = "TcpClient",
    .size = sizeof(TcpClient),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)tcpclient_destroy,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)tcpclient_to_cstr,
    .to_cstr = (to_cstr_cb)tcpclient_to_cstr,
};

const void *TcpClientCls = &_TcpClientCls;
