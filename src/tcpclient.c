#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <masc/tcpclient.h>


#define RX_BUFFER_SIZE 512


static void _dlf_data_cb(TcpClient *self, void *data, size_t len);


void tcpclient_init(TcpClient *self, const char *ip, in_port_t port)
{
    object_init(self, TcpClientCls);
    self->sentinel = '\n';
    self->timeout = 3000;
    self->conn_evt = NULL;
    self->time_evt = NULL;
    self->cli_connect_cb = NULL;
    self->cli_data_cb = _dlf_data_cb;
    self->cli_packet_cb = NULL;
    self->serv_hup_cb = NULL;
    self->fd = -1;
    self->data = NULL;
    self->size = 0;
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

static void _dlf_data_cb(TcpClient *self, void *new_data, size_t new_size)
{
    char *data = new_data;
    size_t size = new_size;
    if (self->data != NULL) {
        // Append new data to existing
        data = self->data = realloc(self->data, self->size + new_size);
        memcpy(self->data + self->size, new_data, new_size);
        size = self->size += new_size;
    }
    // Search for complete packets which end with a sentinel
    size_t i = 0, pos = 0;
    for (i = 0; i < size; i++) {
        if (data[i] == self->sentinel) {
            if (self->cli_packet_cb != NULL) {
                self->cli_packet_cb(self, data + pos, i - pos + 1);
            }
            // Set new start posistion and skip the sentinel
            pos = i + 1;
        }
    }
    // Copy remaining data to client for the next round
    if (pos < i) {
        self->size = i - pos;
        self->data = realloc(self->data, self->size);
        memcpy(self->data, data + pos, self->size);
    } else {
        free(self->data);
        self->data = NULL;
        self->size = 0;
    }
}

static void _client_cb(MlFd *self, int fd, ml_fd_flag_t events, void *arg)
{
    TcpClient *cli = arg;
    if (events & ML_FD_READ) {
        // Read data
        size_t buf_size = 0;
        size_t pos = 0;
        void *buf = NULL;
        while(true) {
            if (buf_size <= pos) {
                buf_size += RX_BUFFER_SIZE;
                buf = realloc(buf, buf_size);
            }
            ssize_t len = read(cli->fd, buf + pos, buf_size - pos);
            if (len > 0) {
                pos += len;
            } else {
                break;
            }
        }
        if(pos > 0 && cli->cli_data_cb != NULL) {
            cli->cli_data_cb(cli, buf, pos);
        }
        free(buf);
    }
    if (events & ML_FD_EOF) {
        // If EOF is indicated shutdown the client
        cli->err = TCPCLIENT_SERV_HUP_ERR;
        if(cli->serv_hup_cb != NULL) {
            cli->serv_hup_cb(cli);
        }
        tcpclient_stop(cli);
    }
}

static void _connect_cb(MlFd *self, int fd, ml_fd_flag_t events, void *arg)
{
    TcpClient *cli = arg;
    mloop_fd_delete(cli->conn_evt);
    cli->conn_evt = NULL;
    int so_err;
    socklen_t so_err_len = sizeof(so_err);
    getsockopt(cli->fd, SOL_SOCKET, SO_ERROR, &so_err, &so_err_len);
    if (cli->cli_connect_cb != NULL) {
        cli->cli_connect_cb(cli, so_err);
    }
    if (so_err == 0) {
        mloop_fd_new(cli->fd, ML_FD_READ, _client_cb, cli);
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
    if (cli->cli_connect_cb != NULL) {
        cli->cli_connect_cb(cli, ETIMEDOUT);
    }
    cli->err = TCPCLIENT_CONNECT_ERR;
    close(cli->fd);
    cli->fd = -1;
    mloop_timer_delete(cli->time_evt);
    cli->time_evt = NULL;
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
        mloop_fd_new(self->fd, ML_FD_READ, _client_cb, self);
    } else if (errno == EINPROGRESS) {
        // Connecting process needs more time
        in_progress = true;
        self->conn_evt = mloop_fd_new(self->fd, ML_FD_WRITE, _connect_cb, self);
        if (self->timeout > 0) {
            self->time_evt = mloop_timer_new(self->timeout, _timeout_cb, self);
        }
    } else {
        self->err = TCPCLIENT_CONNECT_ERR;
        close(self->fd);
        self->fd = -1;
        return self->err;
    }
    if (!in_progress && self->cli_connect_cb != NULL) {
        self->cli_connect_cb(self, errno);
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
    .len = (len_cb)object_len,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)tcpclient_to_cstr,
    .to_cstr = (to_cstr_cb)tcpclient_to_cstr,
};

const void *TcpClientCls = &_TcpClientCls;
