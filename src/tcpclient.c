#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <masc/tcpclient.h>
#include <masc/mloop.h>
#include "mloop/fd.h"


#define RX_BUFFER_SIZE 512


static void _dlf_data_cb(TcpClient *self, void *data, size_t len);


void tcpclient_init(TcpClient *self, const char *ip, in_port_t port)
{
    object_init(self, TcpClientCls);
    self->sentinel = '\n';
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

TcpClientError tcpclient_start(TcpClient *self)
{
    if (self->err != TCPCLIENT_SUCCESS) {
        return self->err;
    }
    if ((self->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        self->err = TCPCLIENT_SOCKET_ERR;
        return self->err;
    }
    if (connect(self->fd, (struct sockaddr *)&self->addr,
            sizeof(self->addr)) < 0) {
        self->err = TCPCLIENT_CONNECT_ERR;
        return self->err;
    }
    //TODO: Connection timeout
    /* Could be done this way ... (of course by using mloop)
     * 
     * The socket is nonblocking and the connection  cannot  be  com‐
     * pleted  immediately.   It  is possible to select(2) or poll(2)
     * for completion by selecting the  socket  for  writing.   After
     * select(2) indicates writability, use getsockopt(2) to read the
     * SO_ERROR option at level SOL_SOCKET to determine whether  con‐
     * nect() completed successfully (SO_ERROR is zero) or unsuccess‐
     * fully (SO_ERROR is one of the usual error codes  listed  here,
     *  explaining the reason for the failure).

    res = connect(soc, (struct sockaddr *)&addr, sizeof(addr)); 
    if (res < 0) { 
     if (errno == EINPROGRESS) { 
        tv.tv_sec = 15; 
        tv.tv_usec = 0; 
        FD_ZERO(&myset); 
        FD_SET(soc, &myset); 
        if (select(soc+1, NULL, &myset, NULL, &tv) > 0) { 
           lon = sizeof(int); 
           getsockopt(soc, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon); 
           if (valopt) { 
              fprintf(stderr, "Error in connection() %d - %s\n", valopt, strerror(valopt)); 
              exit(0); 
           } 
        } 
        else { 
           fprintf(stderr, "Timeout or error() %d - %s\n", valopt, strerror(valopt)); 
           exit(0); 
        } 
     } 
     else { 
        fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
        exit(0); 
     } 
    } 
    */
    mloop_fd_new(self->fd, ML_FD_READ, _client_cb, self);
    return self->err;
}

void tcpclient_stop(TcpClient *self)
{
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
            inet_ntoa(self->addr.sin_addr), ntohs(self->addr.sin_port), self);
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
