#ifndef _MASC_TCPSERVER_H_
#define _MASC_TCPSERVER_H_

#include <netinet/in.h>

#include <masc/object.h>
#include <masc/list.h>


typedef enum {
    TCPSERVER_SUCCESS,
    TCPSERVER_INVALID_ADDR,
    TCPSERVER_SOCKET_ERR,
    TCPSERVER_SETSOCKET_ERR,
    TCPSERVER_BIND_ERR,
    TCPSERVER_LISTEN_ERR,
    TCPSERVER_ERROR
} TcpServerError;

typedef struct TcpServer TcpServer;

typedef struct {
    Object;
    TcpServer *server;
    int fd;
    struct sockaddr_in addr;
    void *data;
    size_t size;
} TcpServerCli;

typedef bool (*tcpserver_accept_cb)(TcpServer *self, TcpServerCli *cli);
typedef void (*tcpserver_data_cb)(TcpServer *self, TcpServerCli *cli,
                                  void *data, size_t size);
typedef void (*tcpserver_hup_cb)(TcpServer *self, TcpServerCli *cli);


struct TcpServer {
    Object;
    int listen_backlog;
    char sentinel;
    struct sockaddr_in addr;
    int fd;
    tcpserver_accept_cb accept_cb;
    tcpserver_data_cb cli_data_cb;
    tcpserver_data_cb cli_packet_cb;
    tcpserver_hup_cb cli_hup_cb;
    List clients;
    TcpServerError err;
};


extern const void *TcpServerCls;


void tcpserver_init(TcpServer *self, const char *ip, in_port_t port);

void tcpserver_destroy(TcpServer *self);

TcpServerError tcpserver_start(TcpServer *self);

void tcpserver_cli_close(TcpServerCli *cli);

size_t tcpserver_to_cstr(TcpServer *self, char *cstr, size_t size);


#endif /* _MASC_TCPSERVER_H_ */
