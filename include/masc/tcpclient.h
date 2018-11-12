#ifndef _MASC_TCPCLIENT_H_
#define _MASC_TCPCLIENT_H_

#include <masc/object.h>
#include <masc/socket.h>
#include <masc/mloop.h>


typedef enum {
    TCPCLIENT_SUCCESS,
    TCPCLIENT_INVALID_ADDR,
    TCPCLIENT_SOCKET_ERR,
    TCPCLIENT_CONNECT_ERR,
    TCPCLIENT_SERV_HUP_ERR,
    TCPCLIENT_ERROR,
} TcpClientError;

typedef struct TcpClient TcpClient;

typedef void (*tcpclient_data_cb)(TcpClient *self, void *data, size_t size);
typedef void (*tcpclient_conn_cb)(TcpClient *self, const char *err_msg);
typedef void (*tcpclient_hup_cb)(TcpClient *self);


struct TcpClient {
    Object;
    char *ip;
    int port;
    Socket *sock;
    char sentinel;
    int timeout;
    MlIo *conn_evt;
    MlTimer *time_evt;
    tcpclient_conn_cb connect_cb;
    tcpclient_data_cb data_cb;
    tcpclient_data_cb pkg_cb;
    tcpclient_hup_cb serv_hup_cb;
    TcpClientError err;
};


extern const void *TcpClientCls;


void tcpclient_init(TcpClient *self, const char *ip, int port);

void tcpclient_destroy(TcpClient *self);

TcpClientError tcpclient_start(TcpClient *self);
void tcpclient_stop(TcpClient *self);

size_t tcpclient_to_cstr(TcpClient *self, char *cstr, size_t size);


#endif /* _MASC_TCPCLIENT_H_ */
