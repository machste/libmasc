#ifndef _MASC_SOCKET_H_
#define _MASC_SOCKET_H_

#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <masc/io.h>
#include <masc/str.h>


typedef struct {
    Io;
} Socket;


extern const io_class *SocketCls;


Socket *socket_new(int family, int type, int protocol);
void socket_init(Socket *self, int family, int type, int protocol);

void socket_destroy(Socket *self);
void socket_delete(Socket *self);

bool socket_connect(Socket *self, const char *ip, int port);

bool socket_bind(Socket *self, const char *ip, int port);

bool socket_listen(Socket *self, int backlog);

Socket *socket_accept(Socket *self, struct sockaddr *addr, socklen_t *addrlen);

int socket_getsockopt(Socket *self, int level, int optname, void *optval,
        socklen_t *optlen);

int socket_setsockopt(Socket *self, int level, int optname, void *optval,
        socklen_t optlen);

#endif /* _MASC_SOCKET_H_ */
