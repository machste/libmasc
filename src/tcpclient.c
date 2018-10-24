#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <masc/tcpclient.h>


void tcpclient_init(TcpClient *self, const char *ip, in_port_t port)
{
    object_init(self, TcpClientCls);
    self->sentinel = '\n';
    self->timeout = 3000;
    self->conn_evt = NULL;
    self->time_evt = NULL;
    self->connect_cb = NULL;
    self->data_cb = NULL;
    self->pkg_cb = NULL;
    self->serv_hup_cb = NULL;
    self->fd = -1;
    self->addr.sin_family = AF_INET;        
    self->addr.sin_port = htons(port);    
    if (inet_aton(ip, &self->addr.sin_addr)) {
        self->err = TCPCLIENT_SUCCESS;
    } else {
        self->err = TCPCLIENT_INVALID_ADDR;
    }
}

static void _vinit(TcpClient *self, va_list va)
{
    char *ip = va_arg(va, char *);
    in_port_t port = (in_port_t)va_arg(va, int);
    tcpclient_init(self, ip, port);
}

void tcpclient_destroy(TcpClient *self)
{
    tcpclient_stop(self);
}

const char *tcpclient_ip(TcpClient *self)
{
    return inet_ntoa(self->addr.sin_addr);
}

in_port_t tcpclient_port(TcpClient *self)
{
    return ntohs(self->addr.sin_port);
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

static void _data_cb(MlFdReader *self, void *data, size_t size, void *arg)
{
    TcpClient *cli = arg;
    cli->data_cb(cli, data, size);
}

static void _pkg_cb(MlFdPkg *self, void *data, size_t size, void *arg)
{
    TcpClient *cli = arg;
    cli->pkg_cb(cli, data, size);
}

static void _eof_cb(MlFdReader *self, void *arg)
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
        mloop_fd_reader_new(self->fd, _data_cb, _eof_cb, self);
    } else if (self->pkg_cb != NULL) {
        mloop_fd_pkg_new(self->fd, self->sentinel, _pkg_cb, _eof_cb, self);
    } else {
        mloop_fd_reader_new(self->fd, NULL, _eof_cb, self);
    }
}

static void _conn_evt(MlFd *self, int fd, ml_fd_flag_t events, void *arg)
{
    TcpClient *cli = arg;
    mloop_fd_delete(cli->conn_evt);
    cli->conn_evt = NULL;
    int so_err;
    socklen_t so_err_len = sizeof(so_err);
    getsockopt(cli->fd, SOL_SOCKET, SO_ERROR, &so_err, &so_err_len);
    _connect_cb(cli, so_err);
    if (so_err == 0) {
        _reg_data_callbacks(cli);
        if (cli->time_evt != NULL) {
            mloop_timer_delete(cli->time_evt);
            cli->time_evt = NULL;
        }
    } else {
        cli->err = TCPCLIENT_CONNECT_ERR;
        close(cli->fd);
        cli->fd = -1;
    }
}

static void _timeout_cb(MlTimer *self, void *arg)
{
    TcpClient *cli = arg;
    mloop_fd_delete(cli->conn_evt);
    cli->conn_evt = NULL;
    mloop_timer_delete(cli->time_evt);
    cli->time_evt = NULL;
    cli->err = TCPCLIENT_CONNECT_ERR;
    _connect_cb(cli, ETIMEDOUT);
    close(cli->fd);
    cli->fd = -1;
}

TcpClientError tcpclient_start(TcpClient *self)
{
    if (self->err != TCPCLIENT_SUCCESS) {
        return self->err;
    }
    if ((self->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        self->err = TCPCLIENT_SOCKET_ERR;
        return self->err;
    }
    mloop_fd_set_blocking(self->fd, false);
    // Try to connect
    bool in_progress = false;
    if (connect(self->fd, (struct sockaddr *)&self->addr,
            sizeof(self->addr)) == 0) {
        _reg_data_callbacks(self);
    } else if (errno == EINPROGRESS) {
        // Connecting process needs more time
        in_progress = true;
        self->conn_evt = mloop_fd_new(self->fd, ML_FD_WRITE, _conn_evt, self);
        if (self->timeout > 0) {
            self->time_evt = mloop_timer_new(self->timeout, _timeout_cb, self);
        }
    } else {
        self->err = TCPCLIENT_CONNECT_ERR;
        close(self->fd);
        self->fd = -1;
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
        mloop_fd_delete(self->conn_evt);
    }
    if (self->time_evt != NULL) {
        mloop_timer_delete(self->time_evt);
    }
    if (self->fd >= 0) {
        MlFd *mlfd = mloop_fd_by_fd(self->fd);
        if (mlfd != NULL) {
            mloop_fd_delete(mlfd);
        }
        close(self->fd);
        self->fd = -1;
    }
}

size_t tcpclient_to_cstr(TcpClient *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s %s:%u at %p>", name_of(self),
            tcpclient_ip(self), tcpclient_port(self), self);
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
