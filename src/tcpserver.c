#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <masc/tcpserver.h>
#include <masc/mloop.h>
#include "mloop/fd.h"


#define RX_BUFFER_SIZE 512

static const void *TcpServerCliCls;


static void _dlf_data_cb(TcpServer *self, TcpServerCli *cli,
        void *data, size_t len);

void tcpserver_init(TcpServer *self, const char *ip, in_port_t port)
{
    object_init(self, TcpServerCls);
    self->listen_backlog = 4;
    self->sentinel = '\n';
    self->accept_cb = NULL;
    self->cli_data_cb = _dlf_data_cb;
    self->cli_packet_cb = NULL;
    self->cli_hup_cb = NULL;
    list_init(&self->clients);
    self->fd = -1;
    self->addr.sin_family = AF_INET;        
    self->addr.sin_port = htons(port);    
    if (inet_aton(ip, &self->addr.sin_addr)) {
        self->err = TCPSERVER_SUCCESS;
    } else {
        self->err = TCPSERVER_INVALID_ADDR;
    }
}

static void _vinit(TcpServer *self, va_list va)
{
    char *ip = va_arg(va, char *);
    in_port_t port = (in_port_t)va_arg(va, int);
    tcpserver_init(self, ip, port);
}

void tcpserver_destroy(TcpServer *self)
{
    list_destroy(&self->clients);
    if (self->fd >= 0) {
        MlFd *mlfd = mloop_fd_by_fd(self->fd);
        if (mlfd != NULL) {
            mloop_fd_delete(mlfd);
        }
        close(self->fd);
    }
}

static void _dlf_data_cb(TcpServer *self, TcpServerCli *cli,
        void *new_data, size_t new_size)
{
    char *data = new_data;
    size_t size = new_size;
    if (cli->data != NULL) {
        // Append new data to existing
        data = cli->data = realloc(cli->data, cli->size + new_size);
        memcpy(cli->data + cli->size, new_data, new_size);
        size = cli->size += new_size;
    }
    // Search for complete packets which end with a sentinel
    size_t i = 0, pos = 0;
    for (i = 0; i < size; i++) {
        if (data[i] == self->sentinel) {
            if (self->cli_packet_cb != NULL) {
                self->cli_packet_cb(self, cli, data + pos, i - pos + 1);
            }
            // Set new start posistion and skip the sentinel
            pos = i + 1;
        }
    }
    // Copy remaining data to cli object for the next round
    if (pos < i) {
        cli->size = i - pos;
        cli->data = realloc(cli->data, cli->size);
        memcpy(cli->data, data + pos, cli->size);
    } else {
        free(cli->data);
        cli->data = NULL;
        cli->size = 0;
    }
}

static void _cli_cb(MlFd *self, int fd, ml_fd_flag_t events, void *arg)
{
    TcpServerCli *cli = arg;
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
        if(pos > 0 && cli->server->cli_data_cb != NULL) {
            cli->server->cli_data_cb(cli->server, cli, buf, pos);
        }
        free(buf);
    }
    if (events & ML_FD_EOF) {
        // If EOF is indicated shutdown the client
        if(cli->server->cli_hup_cb != NULL) {
            cli->server->cli_hup_cb(cli->server, cli);
        }
        tcpserver_cli_close(cli);
    }
}

static void _server_cb(MlFd *self, int fd, ml_fd_flag_t events, void *arg)
{
    TcpServer *server = arg;
    int cli_fd;
    struct sockaddr in_addr;
    socklen_t in_len = sizeof(in_addr);
    while ((cli_fd = accept(self->fd, &in_addr, &in_len)) != -1) {
        TcpServerCli *cli = new(TcpServerCli, server, cli_fd, &in_addr);
        bool accept_cli = true;
        if(server->accept_cb != NULL) {
            accept_cli = server->accept_cb(server, cli);
        }
        if (accept_cli) {
            mloop_fd_new(cli->fd, ML_FD_READ, _cli_cb, cli);
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
    if ((self->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        self->err = TCPSERVER_SOCKET_ERR;
        return self->err;
    }
    int o = 1;
    if (setsockopt(self->fd, SOL_SOCKET, SO_REUSEADDR, &o, sizeof(o)) < 0) {
        close(self->fd);
        self->err = TCPSERVER_SETSOCKET_ERR;
        return self->err;
    }
    if (bind(self->fd, (struct sockaddr *)&self->addr,
            sizeof(self->addr)) < 0) {
        close(self->fd);
        self->err = TCPSERVER_BIND_ERR;
        return self->err;
    }
    if (listen(self->fd, self->listen_backlog) < 0) {
        close(self->fd);
        self->err = TCPSERVER_LISTEN_ERR;
        return self->err;
    }
    mloop_fd_new(self->fd, ML_FD_READ, _server_cb, self);
    return self->err;
}

void tcpserver_cli_close(TcpServerCli *cli)
{
    list_delete_obj(&cli->server->clients, cli);
}

size_t tcpserver_to_cstr(TcpServer *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s %s:%u at %p>", name_of(self),
            inet_ntoa(self->addr.sin_addr), ntohs(self->addr.sin_port), self);
}


static class _TcpServerCls = {
    .name = "TcpServer",
    .size = sizeof(TcpServer),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)tcpserver_destroy,
    .len = (len_cb)object_len,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)tcpserver_to_cstr,
    .to_cstr = (to_cstr_cb)tcpserver_to_cstr,
};

const void *TcpServerCls = &_TcpServerCls;


static void _cli_vinit(TcpServerCli *self, va_list va)
{
    object_init(self, TcpServerCliCls);
    self->server = va_arg(va, TcpServer *);
    self->fd = va_arg(va, int);
    struct sockaddr_in *addr = va_arg(va, struct sockaddr_in *);
    memcpy(&self->addr, addr, sizeof(self->addr));
    self->data = NULL;
    self->size = 0;
}

static void _cli_destroy(TcpServerCli *self, va_list va)
{
    MlFd *mlfd = mloop_fd_by_fd(self->fd);
    if (mlfd != NULL) {
        mloop_fd_delete(mlfd);
    }
    if (self->data != NULL) {
        free(self->data);
    }
    close(self->fd);
}

static size_t _cli_to_cstr(TcpServerCli *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s %s:%u at %p>", name_of(self),
            inet_ntoa(self->addr.sin_addr), ntohs(self->addr.sin_port), self);
}


static class _TcpServerCliCls = {
    .name = "TcpServerCli",
    .size = sizeof(TcpServerCli),
    .vinit = (vinit_cb)_cli_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)_cli_destroy,
    .len = (len_cb)object_len,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)_cli_to_cstr,
    .to_cstr = (to_cstr_cb)_cli_to_cstr,
};

static const void *TcpServerCliCls = &_TcpServerCliCls;
